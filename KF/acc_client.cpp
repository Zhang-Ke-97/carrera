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
#define IP_PI0 "172.20.10.100"
#define IP_MACBOOK "192.168.1.102"

int main(){
    // create socket 
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd<0){
        std::cout << "socket creation failed\n";
        exit(1);
    }

    // set up server address we want to connect to
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DATA_PORT);
    inet_pton(AF_INET, IP_MACBOOK, &server_addr.sin_addr);

    // connect to server
    if(connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        std::cout << "conncetion to server failed\n";
        exit(1);
    }

    // receive acc data from server
    char recv_buffer[1024] = {0};
    std::string command = "get seq";
    while (1){
        send(socket_fd, command.c_str(), command.length(), 0);
        recv(socket_fd, recv_buffer, 1024, 0);
        std::cout << "Data received: " << recv_buffer << '\n';
        usleep(0.1*1000*1000);
    }
    
    return 0;
}