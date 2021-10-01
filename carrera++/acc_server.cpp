/*  Before you complile, install the library:
        sudo apt-get install libi2c-dev
    To compile, run:
        g++ acc_server.cpp mpu6050.cpp -li2c 
*/

#define DATA_PORT 5560 // for socket

#include "mpu6050.h"
#include <iostream>
#include <cstring> // memset
#include <cstdlib> //exit
extern "C" {
    #include <sys/socket.h> // socket
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h> // socketaddr_in
    #include <unistd.h> // sleep
}

std::string read_acc();
std::string read_seq();
static void show_socket_info(struct sockaddr_in *s);

int main(){
    ////////////////////////////// set up i2c bus //////////////////////////////
    // open the i2c bus
    int i2c_fd;
    int adapter_nr = 1; // inspect /sys/class/i2c-dev/ or run "i2cdetect -l"
    char i2c_path[20];
    std::snprintf(i2c_path, 19, "/dev/i2c-%d", adapter_nr);

    i2c_fd = open(i2c_path, O_RDWR);
    if (i2c_fd < 0) {
        std::cout << "Can not open " << i2c_path << "\n";
        exit(1);
    }

    // link the i2c file descriptor to MPU6050
    if (ioctl(i2c_fd, I2C_SLAVE, MPU6050_ADDR) < 0) {
        std::cout << "Unable to link to " << MPU6050_ADDR << "\n";
        exit(1);
    }

    // configure registers in MPU6050 
    config_mpu(i2c_fd);

    ////////////////////////////// set up socket //////////////////////////////
    // create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cout << "socket file descriptor failed to create\n";
        std::exit(1);
    }

    // define the address we want to bind
    struct sockaddr_in pi0addr;  
    pi0addr.sin_family = AF_INET;
    pi0addr.sin_port = htons(DATA_PORT);
    // Use either pi0addr.sin_addr.s_addr = INADDR_ANY;
    //         or inet_pton(AF_INET, "192.168.1.102.", &(pi0addr.sin_addr)); 
    // to bind the socket to a specific IP 
    pi0addr.sin_addr.s_addr = INADDR_ANY;
    //////inet_pton(AF_INET, "192.168.0.10", &pi0addr.sin_addr);
    std::memset(pi0addr.sin_zero, 0, sizeof(pi0addr.sin_zero));

    // bind the socket to the address we specified
    if(bind(socket_fd, (struct sockaddr*)&pi0addr, sizeof(pi0addr)) <0){
        std::cout << "binding failed\n";
        std::exit(1);
    }else{
        std::cout << "Socket created.\n" << "Binding compelte\n";
    }
    
    // listen and accept
    listen(socket_fd, 3); // maximal pending connection =3
    struct sockaddr_in client_addr;  
    int addr_size = sizeof(client_addr);
    int conn_fd = accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addr_size);
    if (conn_fd <0){
        std::cout << "accept() failed\n";
        std::exit(1);
    }else{
        show_socket_info(&client_addr);
    }
    
    ////////////////////////////// Big while loop //////////////////////////////
    double Ax, Ay, Az;
    std::string sep_acc = " ";
    std::string reply = "hello";  // reply to client
    char recv_buffer[1024] = {0}; // received command from client
    std::string command;
    while (1){
        recv(conn_fd, recv_buffer, 1024, 0);
        command = std::string(recv_buffer);
        if (command == "get acc"){
            read_acc(i2c_fd, &Ax, &Ay, &Az);
            reply = std::to_string(Ax) + sep_acc +
                    std::to_string(Ay) + sep_acc + 
                    std::to_string(Az);
        }else if (command == "get seq"){
            reply = read_seq();
        }else{
            std::cout << "Unknown command: " << command << std::endl;
        }

        if (send(conn_fd, reply.c_str(), reply.length(), 0) >0){
            std::cout << "Data has been sent: " << reply << std::endl;
        }
        

    } 

    return 0;
}


std::string read_acc(){
    double acc_x = 0.0;
    double acc_y = 1.0;
    double acc_z = 2.0;

    std::string sep_acc = " ";

    std::string reply = std::to_string(acc_x) + sep_acc +
                        std::to_string(acc_y) + sep_acc + 
                        std::to_string(acc_z);
    return reply;
} 

std::string read_seq(){
    static int seq_counter = 0;
    std::string s = std::to_string(seq_counter++);
    return s;
}

static void show_socket_info(struct sockaddr_in *s) {
    char ip[INET_ADDRSTRLEN];
    uint16_t port;

    inet_ntop(AF_INET, &(s->sin_addr), ip, INET_ADDRSTRLEN);
    port = ntohs(s->sin_port);

    std::cout << "IP: "<< ip << ", port: " << port << std::endl;
}
