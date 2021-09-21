// Registers of MPU6050
#define PWR_MGMT_1    0x6B
#define SMPLRT_DIV    0x19
#define CONFIG        0x1A
#define ACCEL_CONFIG  0x1C
#define INT_ENABLE    0x38
#define ACCEL_XOUT_H  0x3B
#define ACCEL_YOUT_H  0x3D 
#define ACCEL_ZOUT_H  0x3F

#define DATA_PORT 5560 // for socket


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
    pi0addr.sin_addr.s_addr = INADDR_ANY;
    // alternativ:
    // inet_pton(AF_INET, "192.168.1.102.", &(pi0addr.sin_addr)); 
    // The above syntax is used to bind the socket to a specific IP 
    std::memset(pi0addr.sin_zero, 0, sizeof(pi0addr.sin_zero));

    // bind the socket to the address we specified
    if(bind(socket_fd, (struct sockaddr*)&pi0addr, sizeof(pi0addr)) <0){
        std::cout << "binding failed\n";
        std::exit(1);
    }else{
        std::cout << "Socket created.\n" << "Binding compelte\n";
    }
    
    // listen and accept
    listen(socket_fd, 1); // maximal pending connection =1
    struct sockaddr_in client_addr;  
    int addr_size = sizeof(client_addr);
    int conn_fd = accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addr_size);
    if (conn_fd <0){
        std::cout << "accept() failed\n";
        std::exit(1);
    }else{
        show_socket_info(&client_addr);
    }
    
    // send acc data
    std::string reply = "hello";
    char recv_buffer[1024] = {0};
    std::string command;
    while (1){
        recv(conn_fd, recv_buffer, 1024, 0);
        command = std::string(recv_buffer);
        if (command == "get acc"){
            reply = read_acc();
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