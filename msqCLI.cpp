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
    std::string command = "CREATE_QUEUE " + std::string(queue_name) + " " + std::to_string(holding_time);
    send(client_fd, command.c_str(), command.length(), 0);

    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0) {
        if (std::string(buffer).find("QUEUE_CREATED") == 0) {
            return 0;
        }
    }
    return 1;
}

std::string get_available_queues(){
    std::string command = "LIST_QUEUES ";
    send(client_fd, command.c_str(), command.length(), 0);

    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0) {
        if (std::string(buffer).find("AVAILABLE_QUEUES") == 0) {
            return buffer;
        }
    }
    return buffer;

}

// int subscribe(const char* queue_name) {
//     std::string command = "SUBSCRIBE " + std::string(queue_name);
//     send(client_fd, command.c_str(), command.length(), 0);

//     char buffer[BUFFER_SIZE] = {0};
//     int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
//     if (bytes_received > 0) {
//         if (std::string(buffer).find("SUBSCRIBED") == 0) {
//             return 0;
//         }
//     }
//     return 1;
// }

// int unsubscribe(const char* queue_name) {
//     std::string command = "UNSUBSCRIBE " + std::string(queue_name);
//     send(client_fd, command.c_str(), command.length(), 0);

//     char buffer[BUFFER_SIZE] = {0};
//     int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
//     if (bytes_received > 0) {
//         if (std::string(buffer).find("UNSUBSCRIBED") == 0) {
//             return 0;
//         }
//     }
//     return 1;
// }

// int send_msg(const char* queue_name, const char* msg, size_t msg_len) {
//     std::string command = "SEND " + std::string(queue_name) + " " + std::string(msg) + " " + std::to_string(msg_len);
//     send(client_fd, command.c_str(), command.length(), 0);

//     char buffer[BUFFER_SIZE] = {0};
//     int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
//     if (bytes_received > 0) {
//         if (std::string(buffer).find("SEND") == 0) {
//             return 0;
//         }
//     }
//     return 1;
// }

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