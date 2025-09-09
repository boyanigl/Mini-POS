#include <iostream>
#include <vector>
#include <stack>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <poll.h>
#include <sqlite3.h> 
#include <chrono>
#include <thread>

/* local libs*/
#include "Types.h"
#include "Pos.h"


Pos::Pos(){
        // creating socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        // specifying address
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(9000);
        serverAddress.sin_addr.s_addr = inet_addr(LOCAL_HOST);

        struct pollfd fds[1];
        fds[0].fd = clientSocket;
        fds[0].events = POLLIN;  // Wait for incoming data
        int retConnect;

        // sending connection request with RETRY strategy
        for(unsigned attempts = 0; attempts < 3; ++attempts){
            connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
            retConnect = poll(fds, 1, 2000);  // Timeout 2 seconds
            if(retConnect <= 0){
                /* connection is not established*/
                if(attempts == 0){
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));   
                }else if(attempts == 1){
                    std::this_thread::sleep_for(std::chrono::milliseconds(600));                      
                }else{
                    /* max retries reached, go to failure behaviour*/
                }

            }
        }

        if(retConnect <= 0){
            /* Connection Timeout expired */
            std::cout << "Connection Timeout expired\n";
            close(clientSocket);
        }
        else if(clientSocket < 0){
            /* Error connecting */
            close(clientSocket);
            std::cerr << "Error creating socket\n";
        }else{
            /* Connection is established !*/        
        }
    }

    
Pos::Pos(std::string address, unsigned short port){
        // creating socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        // specifying address
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = inet_addr(address.c_str());

        // sending connection request
        connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        if(clientSocket < 0){
        std::cerr << "Error creating socket\n";
        }
    }

Pos::~Pos() {
        close(clientSocket);
    }

retType Pos::Handshake(){
        const char* message = "HELLO|GW|1.0\n";
        char buffer[1024];
        send(clientSocket, message, strlen(message), 0);
        /* wait N s */
        struct pollfd fds[1];
        fds[0].fd = clientSocket;
        fds[0].events = POLLIN;  // Wait for incoming data

        int ret = poll(fds, 1, 5000);  // Timeout 5 seconds
        if(ret <= 0){
            /* Timeout expired or error through the comm*/
            std::cout << "Timeout expired, no response from server\n";
            close(clientSocket);
            return E_NOT_OK;
        }
        int lenRespMsg = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if(lenRespMsg){
            buffer[lenRespMsg] = '\0'; // Null-terminate the received string
            std::string response(buffer);
            if (response.find("HELLO|TERM|1.0") != std::string::npos) {
                std::cout << "Handshake successful\n";
            } else {
                std::cout << "Unexpected response\n";
                return E_NOT_OK;
            }
        }

        return E_OK;
    }

retType Pos::PushSale(char* price, std::time_t *time){
        // current time point
        auto now = std::chrono::system_clock::now();
        // convert to time_t which is a type representing the system time
        *time = std::chrono::system_clock::to_time_t(now);

        char message[50] = "SALE|";
        strcat(message, price);

        std::string unixStr = std::to_string(*time);
        const char* cstr = unixStr.c_str();  
        strcat(message, cstr);
        strcat(message,"|RANDOM_VALUE|\n");


        send(clientSocket, message, strlen(message), 0);
        /* wait N s */
        struct pollfd fds[1];
        fds[0].fd = clientSocket;
        fds[0].events = POLLIN;  // Wait for incoming data

        int ret = poll(fds, 1, 5000);  // Timeout 5 seconds
        if(ret <= 0){
            /* Timeout expired or error through the comm*/
            std::cout << "Timeout expired, no response from server\n";
            close(clientSocket);
            return E_NOT_OK;
        }
        char buffer[512];
        int lenRespMsg = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if(lenRespMsg){
            buffer[lenRespMsg] = '\0'; // Null-terminate the received string
            std::string response(buffer);
            if (response.find("APPROVED") != std::string::npos) {
                std::cout << "Sale is received and approved by server\n";
                    /* store the sale into the local stack of sales*/
                    //sales.push(message);
            } else {
                std::cout << "Unexpected or negative response from server\n";
                return E_NOT_OK;
            }
        }
        return E_OK;

    }
