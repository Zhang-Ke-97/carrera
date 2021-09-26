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

#define DATA_PORT 5560
#define IP_SERVER_PI "172.20.10.100"


int main(){
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
    /*
    if(inet_pton(AF_INET, IP_SERVER_PI, &server_addr.sin_addr)<=0){
	std::cout << "Invalid server address\n";
    }
    */
    server_addr.sin_addr.s_addr = inet_addr(IP_SERVER_PI);
    // connect to server
    if(connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        std::cout << "conncetion to server failed\n";
        exit(1);
    }

    // receive buffer of socket
    char recv_buffer[1024] = {0};
    // command to be sent to server
    std::string command = "get seq";
    
    // store the acc data in double
    double acc_x, acc_y, acc_z;
    // TODO: set up Kalman Filter

    // Big while-loop
    while (1){
        // send the command and get reply from server
        send(socket_fd, command.c_str(), command.length(), 0);
        recv(socket_fd, recv_buffer, 1024, 0);
        std::cout << "Data received: " << recv_buffer << '\n';

        // TODO: convert the received data into double

        // TODO: send the acc into Kalman Filter
        usleep(0.1*1000*1000);
    }
    
    return 0;
}
