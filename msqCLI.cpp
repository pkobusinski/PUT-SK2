#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "msqCLI.hpp"

#define BUFFER_SIZE 1024

static int client_fd = -1;

int connect_to_server(const char* ip, int port) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        //perror("Socket creation error");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        //perror("Connection failed");
        return 1;
    }

    return 0;
}

int disconnect() {
    if (client_fd >= 0) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        client_fd = -1;
        return 0;
    }else   
        return 1;
}

int create_queue(const char* queue_name, int holding_time) {
    fbs action;
    fsb answer;
    action.command = CREATE_QUEUE;
    strcpy(action.queue_name, queue_name); 
    action.holding_time = holding_time;
    
    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

int subscribe(const char* queue_name) {
    fbs action;
    fsb answer;
    action.command = SUBSCRIBE;
    strcpy(action.queue_name, queue_name); 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

int unsubscribe(const char* queue_name) {
    fbs action;
    fsb answer;
    action.command = UNSUBSCRIBE;
    strcpy(action.queue_name, queue_name); 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

int send_msg(const char* queue_name, const char* msg, size_t msg_len) {
    fbs action;
    fsb answer;
    action.command = SEND;
    strcpy(action.queue_name, queue_name); 
    strcpy(action.message, msg);
    action.msg_len = msg_len; 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

// int recv_msg(const char* queue_name, const char* msg, size_t msg_len) {
//     std::string command = "RECV " + std::string(queue_name) + " " + std::string(msg) + " " + std::to_string(msg_len);
//     send(client_fd, command.c_str(), command.length(), 0);

//     char buffer[BUFFER_SIZE] = {0};
//     int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
//     if (bytes_received > 0) {
//         if (std::string(buffer).find("RECV") == 0) {
//             return 0;
//         }
//     }
//     return 1;
// }

// std::string get_available_queues(){
//     std::string command = "LIST_QUEUES ";
//     send(client_fd, command.c_str(), command.length(), 0);


//     char buffer[BUFFER_SIZE] = {0};
//     int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
//     if (bytes_received > 0) {
//         if (std::string(buffer).find("AVAILABLE_QUEUES") == 0) {
//             return buffer;
//         }
//     }
//     return buffer;

// }