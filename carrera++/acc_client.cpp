/* To compile, run
    g++ acc_client.cpp KF.cpp -o ../build/acc_client -I /usr/local/include/eigen3 -Wno-psabi -lpthread -lpigpio -lrt
*/
#include <iostream>
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

#define DATA_PORT 5560                   // for socket
#define IP_SERVER_PI "192.168.1.200"     // for socket
#define LENGTH(a) sizeof(a)/sizeof(a[0]) // easy to handle array length
#define GRAVITY_STG 9.80884              // gravitation in Stuttgart (in N/kg)
#define CARRERA_BAHN_LENGTH 1.8          // length of Carrera-Bahn (in m)
#define GATE_DISTENCE 0.15               // distance between lasers (in m)
#define GPIO_GATE_1 14                   // GPIO Pin for light sensor at position 1
#define GPIO_GATE_2 15                   // GPIO Pin for light sensor at position 2

#define ADD_KALMAN

// save accel, velo, etc in Dashboard
Dashboard dsb;

/* Helper: extracts doubles from the string we received
    s: the C string from which we would like to extract doubles
    len: length of the C string s
    x, y, z: extracted information will be stored in these vaiables 
*/
static void extract_acc_from_string(char* s, int len, double &x, double &y, double &z);

/* ISR call back function associated gate 1 */
void gate1_ISR_callback(int gpio, int level, uint32_t tick);

/* ISR call back function associated gate 2 */
void gate2_ISR_callback(int gpio, int level, uint32_t tick);

// set up Kalman Filter
const double T_sample = 0.1;    // sampling periode 

// open csv file
std::ofstream training_data("train.csv");
 
Matrix A { // state transistion mtx
    {1.0,  T_sample, T_sample*T_sample/2},
    {0.0,  1.0,      T_sample}, 
    {0.0,  0.0,      0.0}
};

Matrix B { // control input mtx
    {0.0},
    {0.0},
    {1.0}
};

Matrix C { // measurement output mtx
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};

Matrix Q { // model noise cov
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
}; 

Matrix R { // measurement noise cov
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
}; 

Matrix P { // initial error cov
    {1000.0,    0.0,      0.0},
    {0.0,    1000.0,      0.0},
    {0.0,       0.0,   1000.0}
}; 

Vector x { // initial state estimate
    {0.0}, // mileage
    {0.0}, // velo
    {0.0}  // accel
}; 

Vector u { // control input
    {0.0}
}; 

Vector z { // measurements
    {0.0},
    {0.0},
    {0.0}
};

KF kf = KF(A, B, C, Q, R, P, x, u, z);    

int main(){
    ///////////////////////////////// write header in .csv file /////////////////////////////////    
    training_data << "accel x" << "," << "accel y" << "," << "accel z" << "," 
              << "velocity tg" << "," << "mileage tg" << "," << "width PWM" << "\n";

    ///////////////////////////////// set up GPIO & interrupt /////////////////////////////////
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

    gpioSetISRFunc(GPIO_GATE_1, RISING_EDGE, -1, gate1_ISR_callback);
    gpioSetISRFunc(GPIO_GATE_2, RISING_EDGE, -1, gate2_ISR_callback);

    ///////////////////////////////// set up socket /////////////////////////////////
    // create socket 
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd<0){
        std::cout << "socket creation failed\n";
        exit(1);
    }

    // set up server address (and the port number) we want to connect to
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DATA_PORT);
    // Use either server_addr.sin_addr.s_addr = inet_addr(IP_SERVER_PI);
    //         or inet_pton(AF_INET, IP_SERVER_PI, &server_addr.sin_addr); 
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

    #ifndef ADD_KALMAN
    training_data.close();
    #endif
    
    ///////////////////////////////// Big while-loop /////////////////////////////////
    while (1){
        // send the command and get reply from server
        send(socket_fd, command.c_str(), command.length(), 0);
        recv(socket_fd, recv_buffer, 1024, 0);

        // extract acceleration (normalized to g) from recveive buffer
        extract_acc_from_string(recv_buffer, LENGTH(recv_buffer), dsb.acc_x, dsb.acc_y, dsb.acc_z);
        std::cout << "Received accel: " << "Ax=" << dsb.acc_x << "g, "
                                        << "Ay=" << dsb.acc_y << "g, "
                                        << "Az=" << dsb.acc_z << "g\n";
        // convert acceleration to N/kg
        dsb.acc_x *= GRAVITY_STG;
        dsb.acc_y *= GRAVITY_STG;
        dsb.acc_z *= GRAVITY_STG;
        
        #ifdef ADD_KALMAN
        // perform kalman filtering
        kf.predict();

        // save prior state estimate in vector x
        x << kf.get_prio_state_estm(); 
        // save features in .csv file
        training_data << dsb.acc_x << "," << dsb.acc_y << "," << dsb.acc_z << "," 
                      << x(1) << "," << x(0) << "," << "width PWM" << "\n";
        #endif
        
        usleep(0.1*1000*1000);
    }
    
    return 0;
}



// extract doubles from the string we received
// s: the C string from which we would like to extract doubles
// len: length of the C string s
// x, y, z: extracted information will be stored in these vaiables 
static void extract_acc_from_string(char* s, int len, double &x, double &y, double &z){
    std::string str(s, len);
    std::stringstream ss(str);
    ss >> x;
    ss >> y;
    ss >> z;
}

// ISR call back function associated gate 1
void gate1_ISR_callback(int gpio, int level, uint32_t tick){
    // set timer and accumulate cycle
    dsb.set_t_gate1();
    dsb.cycle++;
    std::cout << "\n" << "car arrived at gate 1, " << "cycle=" << dsb.cycle << "\n\n";
}

// ISR call back function associated gate 2
void gate2_ISR_callback(int gpio, int level, uint32_t tick){
    // set timer and compute velocity
    dsb.set_t_gate2();
    dsb.get_velocity(GATE_DISTENCE);

    std::cout << "\n" << "car arrived at gate 2, " 
              << "delta_t=" << dsb.t_gate2 - dsb.t_gate1 << "\n\n";

    
    std::cout << -1 << ", " << "posterior estimate" << "\n";


    #ifdef ADD_KALMAN
    // kalman filter: update()
    z << dsb.mileage + GATE_DISTENCE, dsb.velo, dsb.acc_y;
    kf.update(z);

    // save posterior state estimate in vector x
    x << kf.get_post_state_estm(); 
    // save features in .csv file
    training_data << dsb.acc_x << "," << dsb.acc_y << "," << dsb.acc_z << "," 
                  << x(1) << "," << x(0) << "," << "width PWM" << "\n";

    // some outputs
    Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); // set matrix print-layout
    std::cout << "\n" << "car arrived at gate 2\n";
    std::cout << "state estimate:\n" << x.format(fmt) << "\n";
    std::cout << "error covariance:\n" << kf.get_error_covariance().format(fmt) << "\n\n";
    #endif
}
