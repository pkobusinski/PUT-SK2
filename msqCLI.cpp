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
        std::cerr << "Socket creation error\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server.\n";
    return 0;
}

int disconnect() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    return 0;
}

int create_queue(const char* queue_name, int holding_time) {
    std::string command = "CREATE_QUEUE " + std::string(queue_name) + " " + std::to_string(holding_time);
    send(client_fd, command.c_str(), command.length(), 0);

    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0) {
        if (std::string(buffer).find("QUEUE_CREATED") != 0) {
            return 0;
        }
    }
    return 1;
}
