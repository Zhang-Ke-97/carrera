/**
 * @file acc_client.cpp
 * @brief Main file for reveiveing acceleration data from Pi 0 and feed the data into Kalman Filter. 
 * @author Ke Zhang
 * @date 31. October 2021
 * 
 * To compile, run
 * g++ acc_client.cpp KF.cpp -o ../build/acc_client -I /usr/include/eigen3 -Wno-psabi -lpthread -lpigpio -lrt
*/
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstring> // memset
#include <cstdlib> //exit
#include <ctime>
#include "KF.h"
#include "dashboard.h"
extern "C" {
    #include <sys/socket.h> // socket
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h> // socketaddr_in
    #include <unistd.h>     // sleep
    #include <pigpio.h>     // GPIO Lib
}

#define DATA_PORT 5560                   /** @brief for socket */
#define IP_SERVER_PI "192.168.1.200"     /** @brief for socket */

#define LENGTH(a) sizeof(a)/sizeof(a[0]) /** @brief easy to handle array length */

#define GRAVITY_STG 9.80884              /** @brief gravitation in Stuttgart (in N/kg) */
#define CARRERA_BAHN_LENGTH 4.583        /** @brief length of Carrera-Bahn (in m) */
#define GATE_DISTENCE 0.06               /** @brief distance between lasers (in m) */

#define GPIO_GATE_1 14                   /** @brief GPIO Pin for light sensor at position 1 */
#define GPIO_GATE_2 15                   /** @brief GPIO Pin for light sensor at position 2 */

#define ADD_KALMAN
// #define WRITE_FEATURES

// save accel, velo, etc in Dashboard
Dashboard dsb;

// sampling periode (in s)
const double T_sample = 0.1;    

// open csv file
std::ofstream training_data("train.csv");

// Kalman Filter
KF kf(3,3,1); // dim_x, dim_z, dim_u

// saves prior/posterior state estimate
Vector state_tg(3);

// saves measurements
Vector measm(3);

// set matrix print-layout
Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); 

/* Helper: extracts doubles from the string we received
    s: the C string from which we would like to extract doubles
    len: length of the C string s
    x, y, z: extracted information will be stored in these vaiables 
*/
/**
 * @brief Helper: extracts doubles from a string 
 * @param s the string which containing the doubles 
 * @param len length of the string s
 * @param x reference to variable containing acceleration in x direction
 * @param y reference to variable containing acceleration in y direction
 * @param z reference to variable containing acceleration in z direction
 * @return none
 */
static void str_to_doubles(char* s, int len, double &x, double &y, double &z);

/**
 * @brief Helper: extracts doubles from a string 
 * @param len length of the string s
 * @param dsb reference to struct Dashboard containing acceleration in x, y and z direction 
 * @return none
 */ 
static void str_to_accel(char* s, int len, Dashboard &dsb);

/**
 * @brief Helper: Write features in .csv file (accel, velo, etc.)
 * @param output_file The file which saves the features
 * @return none
 */
static void save_features(std::ofstream output_file);

/**
 * @brief Helper: Shows the features to console (accel, velo, etc.) to console
 * @param s additional information (optional)
 */
static void show_features(const char* s = "\0");
 
 
/**
 * @brief Interrupt service routine (ISR) associated gate 1 
 * 
 * This function will be called if the car passes through gate 1
 * @param gpio GPIO which the ISR is associated to
 * @param level Falling or rising edge
 * @param tick The number of microseconds since boot
 * @return none
 */
void gate1_ISR_callback(int gpio, int level, uint32_t tick);

/**
 * @brief Interrupt service routine (ISR) associated gate 2
 * 
 * This function will be called if the car passes through gate 2
 * @param gpio GPIO which the ISR is associated to
 * @param level Falling or rising edge
 * @param tick The number of microseconds since boot
 * @return none
 */
void gate2_ISR_callback(int gpio, int level, uint32_t tick);

int main(){
    // sampling periode 
    const double T_sample = 0.1;  

    // state transistion mtx 
    Matrix A(3,3);
    A << 1.0, T_sample, T_sample*T_sample/2,
         0.0,      1.0,            T_sample, 
         0.0,      0.0,                 0.0;

    // control input mtx
    Matrix B(3,1);
    B << 0.0,
         0.0,
         1.0;
         
    // measurement output mtx
    Matrix C(3,3);
    C << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;

    // model noise cov
    Matrix Q(3,3);
    Q << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;

    // measurement noise cov
    Matrix R(3,3);
    R << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;
    
    // control input: accel in z-direction
    Matrix Acc_tg(1,1);
    Acc_tg << 0;
    
    // set up Kalman Filter
    kf.set_up_model(A, B, C);    
    kf.set_model_noise(Q);
    kf.set_measure_noise(R);

    ///////////////////////////////// set up GPIO /////////////////////////////////
    // Light sensor: output = 1 when dark  <=> car arrived
    //                      = 0 when light <=> no car
    // GPIO input: PULL_DOWN required
    if (gpioInitialise()<0){
        std::cout << "Failed to initialise GPIO\n";
        exit(1);
    }
    gpioSetMode(GPIO_GATE_1, PI_INPUT);
    gpioSetMode(GPIO_GATE_2, PI_INPUT);

    gpioSetPullUpDown(GPIO_GATE_1, PI_PUD_DOWN); 
    gpioSetPullUpDown(GPIO_GATE_2, PI_PUD_DOWN); 

    // disable timeout for both ISR by setting timeout = -1
    gpioSetISRFunc(GPIO_GATE_1, RISING_EDGE, -1, gate1_ISR_callback);
    gpioSetISRFunc(GPIO_GATE_2, RISING_EDGE, -1, gate2_ISR_callback);

    ///////////////////////////////// set up socket /////////////////////////////////
    // create socket 
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd<0){
        std::cout << "socket creation failed\n";
        exit(1);
    }

    // set up server address (and the port number) we want to connect to.
    // Use either server_addr.sin_addr.s_addr = inet_addr(IP_SERVER_PI);
    //         or inet_pton(AF_INET, IP_SERVER_PI, &server_addr.sin_addr); 
    // to set up sin_addr
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DATA_PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_SERVER_PI);

    // connect to server
    if(connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        std::cout << "conncetion to server failed\n";
        exit(1);
    }

    // receive buffer of socket
    char recv_buffer[1024] = {0};

    // command to be sent to server, possible choices: "get acc", "get seq"
    std::string command = "get acc";

    #ifndef WRITE_FEATURES
    training_data.close();
    #endif
    
    ///////////////////////////////// Big while-loop /////////////////////////////////
    while (1){
        // request accel data from server
        send(socket_fd, command.c_str(), command.length(), 0);
        recv(socket_fd, recv_buffer, 1024, 0);

        // extract acceleration (normalized to g) from recveive buffer
        str_to_doubles(recv_buffer, LENGTH(recv_buffer), dsb.acc_x, dsb.acc_y, dsb.acc_z);
        std::cout << "Received accel: " << "Ax=" << dsb.acc_x << "g, "
                                        << "Ay=" << dsb.acc_y << "g, "
                                        << "Az=" << dsb.acc_z << "g\n";
        // convert acceleration to N/kg
        dsb.acc_x *= -GRAVITY_STG;
        dsb.acc_y *= -GRAVITY_STG;
        dsb.acc_z *= -GRAVITY_STG;
        
        // perform kalman filtering
        Acc_tg << dsb.acc_y;
        kf.predict(Acc_tg);
        kf.update();

        // save prior state estimate in vector x
        state_tg << kf.get_prio_state_estm(); 

        #ifdef WRITE_FEATURES
        // save features in .csv file
        save_features(training_data);
        #endif
        show_features("Predict: ");
	std::cout << std::endl;

        usleep(T_sample*1000*1000);
    }
    
    return 0;
}




static void str_to_doubles(char* s, int len, double &x, double &y, double &z){
    std::string str(s, len);
    std::stringstream ss(str);
    ss >> x;
    ss >> y;
    ss >> z;
}

static void str_to_accel(char* s, int len, Dashboard &dsb){
    std::string str(s, len);
    std::stringstream ss(str);
    ss >> dsb.acc_x;
    ss >> dsb.acc_x;
    ss >> dsb.acc_x; 
}

void gate1_ISR_callback(int gpio, int level, uint32_t tick){
    // check if the car really arrived at gate 1
    if(clock()-dsb.t_gate1 < CLOCKS_PER_SEC/10){ // divided by 10: experimental
    	return;
    }
    // set timer and accumulate cycle
    dsb.set_t_gate1();
    dsb.cycle++;
    dsb.compute_mileage(CARRERA_BAHN_LENGTH);
    std::cout << "\n" << "car arrived at gate 1, " << "cycle=" << dsb.cycle << "\n\n";
}

void gate2_ISR_callback(int gpio, int level, uint32_t tick){
    // check if the car really arrived at gate 2
    if(clock()-dsb.t_gate2 < CLOCKS_PER_SEC/10){ // divided by 10: experimental
    	return;
    }
    // set timer and compute velocity
    dsb.set_t_gate2();
    dsb.compute_velocity(GATE_DISTENCE);

    #ifdef ADD_KALMAN
    // kalman filter: update()
    measm << dsb.mileage + GATE_DISTENCE, dsb.velo, dsb.acc_z;
    kf.update(measm);

    // save posterior state estimate in vector x
    state_tg << kf.get_post_state_estm(); 
    
    #ifdef WRITE_FEATURES
    // save features in .csv file
    save_features(training_data);
    #endif

    // some outputs
    std::cout << "\n" << "car arrived at gate 2, " 
              << "delta_t=" << dsb.t_gate2 - dsb.t_gate1 << "\n";
    show_features("Update: ");
    #endif
}


static void show_features(const char* s /* ="\0" */){
    std::cout << s
             << "Ax=" << std::setprecision (3) << dsb.acc_x << "m/s^2, "
             << "Ay=" << std::setprecision (3) << dsb.acc_y << "m/s^2, " 
             << "Az=" << std::setprecision (3) << dsb.acc_z << "m/s^2, " 
             << "Vy=" << std::setprecision (3) << state_tg(1) << "m/s, " 
             << "Sy=" << std::setprecision (3) << state_tg(0) << "m, " 
             << "width PWM" << "\n";
}

static void save_features(std::ofstream output_file){
    output_file << dsb.acc_x << "," << dsb.acc_y << "," << state_tg(2) << "," 
                  << state_tg(1) << "," << state_tg(0) << "," << "width PWM" << "\n";
}
