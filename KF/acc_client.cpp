#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring> // memset
#include <cstdlib> //exit
#include <ctime>
#include "KF.h"
extern "C" {
    #include <sys/socket.h> // socket
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h> // socketaddr_in
    #include <unistd.h> // sleep
}

#define DATA_PORT 5560                   // for socket
#define IP_SERVER_PI "172.20.10.100"     // for socket
#define LENGTH(a) sizeof(a)/sizeof(a[0]) // easy to handle array length
#define GRAVITY_STG 9.80884              // gravitation in Stuttgart (in N/kg)
#define CARRERA_BAHN_LENGTH 1.8          // length of Carrera-Bahn (in m)
#define GATE_DISTENCE 0.15               // distance between lasers (in m)

// extract doubles from the string we received
// s: the C string from which we would like to extract doubles
// len: length of the C string s
// x, y, z: extracted information will be stored in these vaiables 
static void extract_acc_from_string(char* s, int len, double &x, double &y, double &z);

// return the mileage of the car at the 1st laser gate
// set the timer t_start for calculating velocity
static double read_mileage(clock_t *t_start);

// return velocity of the car at the 2nd laser gate
static double read_velocity(clock_t t_start);

int main(){
    ///////////////////////////////// set up Kalman Filter /////////////////////////////////
    // sampling periode 
    const double T_sample = 0.1;  
    // state transistion mtx 
    Matrix A {
        {1.0,  T_sample, T_sample*T_sample/2},
        {0.0,  1.0,      T_sample}, 
        {0.0,  0.0,      0.0}
    };
    // control input mtx
    Matrix B {
        {0.0},
        {0.0},
        {1.0}
    };
    // measurement output mtx
    Matrix C {
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
   // model noise cov
    Matrix Q {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    }; 
    // measurement noise cov
    Matrix R {
        {1.0, 0.0},
        {0.0, 1.0}
    }; 
    // initial error cov
    Matrix P {
        {1000.0,    0.0,      0.0},
        {0.0,    1000.0,      0.0},
        {0.0,       0.0,   1000.0}
    }; 
    // initial state estimate
    Vector x {
        {0.0},
        {0.0},
        {0.0}
    }; 
    // control input
    Vector u {
        {0.0}
    }; 
    // measurements
    Vector z {
        {0.0},
        {0.0}
    };
    
    KF kf = KF(A, B, C, Q, R, P, x, u, z);    

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
    if(inet_pton(AF_INET, IP_SERVER_PI, &server_addr.sin_addr)<=0){
	std::cout << "Invalid server address\n";
    }

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
    // store the acc data in double
    double acc_x, acc_y, acc_z;

    ///////////////////////////////// open csv file /////////////////////////////////
    // std::ofstream training_data("train.csv");
    // training_data << "accel x" << "accel y" << "accel z" << "velocity tg" << "mileage tg" << "motor input" << "\n";

    ///////////////////////////////// Big while-loop /////////////////////////////////
    while (1){
        // send the command and get reply from server
        send(socket_fd, command.c_str(), command.length(), 0);
        recv(socket_fd, recv_buffer, 1024, 0);

        // extract acceleration (normalized to g) from recveive buffer
        extract_acc_from_string(recv_buffer, LENGTH(recv_buffer), acc_x, acc_y, acc_z);
        std::cout << "Received accel: " << "Ax=" << acc_x << "g, "
                                        << "Ay=" << acc_y << "g, "
                                        << "Az=" << acc_z << "g\n";
        // convert acceleration to N/kg
        acc_x *= GRAVITY_STG;
        acc_y *= GRAVITY_STG;
        acc_z *= GRAVITY_STG;
        
        // perform kalman filtering
        // kf.predict();

        // Vector measurement {
        //     {read_mileage()},
        //     {read_velocity()}
        // };
        // kf.update(measurement);
        
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

// return the mileage of the car at the 1st laser gate
// set the timer t_start for calculating velocity
static double read_mileage(clock_t *t_start){
    static int cycles = 0; // Number of cycles
    cycles ++;
    *t_start = clock();
    return cycles*CARRERA_BAHN_LENGTH;
}

// return velocity of the car at the 2nd laser gate
static double read_velocity(clock_t t_start){
    double duration_in_sec = double(clock ()-t_start)/CLOCKS_PER_SEC;
    return GATE_DISTENCE/duration_in_sec;
}