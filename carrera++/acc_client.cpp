/* To compile, run
    g++ acc_client.cpp KF.cpp -o acc_client.cpp -I /usr/local/include/eigen3 -Wno-psabi -lpthread -lpigpio -lrt
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring> // memset
#include <cstdlib> //exit
#include <ctime>
#include <thread> 
#include <mutex>
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
#define GPIO_GATE_1 14                   // GPIO Pin for light sensor position 1

// true if car passing gate 1, false else. Shared by threads!
bool signal_gate1 = false;
// mutices to protect global variables shared by threads
std::mutex signal_gate1_mutex;


// Helper: extracts doubles from the string we received
// s: the C string from which we would like to extract doubles
// len: length of the C string s
// x, y, z: extracted information will be stored in these vaiables 
static void extract_acc_from_string(char* s, int len, double &x, double &y, double &z);

// call back function associated with the thread which meusures the light at gate 1
void gate1_callback();

int main(){
    ///////////////////////////////// set up Kalman Filter /////////////////////////////////
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
    
    // define Kalman Filter
    KF kf = KF(3,3,1); // dim_x, dim_z, dim_u
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
    gpioSetPullUpDown(GPIO_GATE_1, PI_PUD_DOWN); 

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

    ///////////////////////////////// open csv file /////////////////////////////////
    std::ofstream training_data("train.csv");
    training_data << "accel x" << "," << "accel y" << "," << "accel z" << "," 
                  << "velocity tg" << "," << "mileage tg" << "," << "width PWM" << "\n";
    #ifndef ADD_KALMAN
    training_data.close();
    #endif
    
    ///////////////////////////////// set up Dashboard ///////////////////////////////// 
    Dashboard dsb;

    ///////////////////////////////// start thread /////////////////////////////////
    std::thread read_gate1(gate1_callback);

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
        #endif

        // check if car arrived at gate 1
        {   
            std::lock_guard<std::mutex> signal_guard(signal_gate1_mutex);
            if(signal_gate1 == true){
                dsb.cycle++;
                dsb.set_t_gate1();
                signal_gate1 = false;
                std::cout << "car arrived at gate 1, " << "cycle=" << dsb.cycle << "\n";
            }
        } // mutices released here

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

// call back function associated with the thread which meusures the light at gate 1
void gate1_callback(){   
    bool cool_down = false; 
    // Keep reading
    while (1){
        {
            std::lock_guard<std::mutex> counter_guard(signal_gate1_mutex);
            if (gpioRead(GPIO_GATE_1) == 1){ // laser is blocked by the car
                signal_gate1 = true;
                cool_down = true;
            }
        } // mutex released
        if (cool_down){
            usleep(2*1000*1000); // avoid repeated "signal arrived"
            cool_down = false;
        }
    }
}
