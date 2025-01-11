#include "msqCLI.hpp"
#include "global.hpp"

#include <string>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>



static int client_fd = -1;

int connect_to_server(const char* ip, int port) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return 1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
    } else {
        return 1;
    }
}

int create_queue(const std::string& queue_name, int holding_time) {
    if (!queue_name.empty() && holding_time > 0) {
        std::string message;
        std::string div = ":";

        message = queue_name + div + std::to_string(holding_time);
        std::string message_content = message.c_str();
        std::string header = create_header(CREAT, message_content.length());

        std::string complete_message = header + message_content; 
        send(client_fd, complete_message.c_str(), complete_message.length(), 0);


        char header_buffer[HEADER_SIZE];
        MsgType result;
        size_t response_length;
        int counter = 0;
        while (counter < HEADER_SIZE) {
            int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
            counter += header_bytes;
        }
        if (!parseHeader(header_buffer, result, response_length)) {
            printf("Invalid header from server \n");
            return 1;
        }

        if(result == SUCCESS)
            return 0;

    }
    return 1;
}

int subscribe(const std::string& queue_name) {
    if (!queue_name.empty()) {
        std::string message;
        std::string div = ":";

        message = queue_name + div;
        std::string message_content = message.c_str();
        std::string header = create_header(SUBSC, message_content.length());

        std::string complete_message = header + message_content; 
        send(client_fd, complete_message.c_str(), complete_message.length(), 0);

        char header_buffer[HEADER_SIZE];
        MsgType result;
        size_t response_length;
        int counter = 0;
        while (counter < HEADER_SIZE) {
            int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
            counter += header_bytes;
        }
        if (!parseHeader(header_buffer, result, response_length)) {
            printf("Invalid header from server \n");
            return 1;
        }

        if(result == SUCCESS)
            return 0;

    }
    return 1;
}

int unsubscribe(const std::string& queue_name) {
    if (!queue_name.empty()) {
        std::string message;
        std::string div = ":";

        message = queue_name + div;
        std::string message_content = message.c_str();
        std::string header = create_header(UNSUB, message_content.length());

        std::string complete_message = header + message_content; 
        send(client_fd, complete_message.c_str(), complete_message.length(), 0);

        char header_buffer[HEADER_SIZE];
        MsgType result;
        size_t response_length;
        int counter = 0;
        while (counter < HEADER_SIZE) {
            int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
            counter += header_bytes;
        }
        if (!parseHeader(header_buffer, result, response_length)) {
            printf("Invalid header from server \n");
            return 0;
        }

        if(result == SUCCESS)
            return 1;

    }
    return 1;
}

int send_msg(const std::string& queue_name, const std::string& msg) {

    std::string message;
    std::string div = ":";

    message = queue_name + div + msg;
    std::string message_content = message.c_str();

    std::string header = create_header(SENDM, message_content.length());
    
    std::string complete_message = header + message_content; 
    send(client_fd, complete_message.c_str(), complete_message.length(), 0);

    char header_buffer[HEADER_SIZE];
    MsgType result;
    size_t response_length;
    int counter = 0;
    while (counter < HEADER_SIZE) {
        int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
        counter += header_bytes;
    }
    if (!parseHeader(header_buffer, result, response_length)) {
        printf("Invalid header from server \n");
        return 1;
    }

    if(result == SUCCESS) {
        std::vector<char> message_buf(response_length);
        int counter = 0;
        while (counter < response_length) {
            int response_bytes = recv(client_fd, message_buf.data() + counter, response_length - counter, 0);
            counter += response_bytes;
        }
        std::string sent_bytes = std::string(message_buf.begin(), message_buf.end());
        return stoi(sent_bytes);
    }

    return 1;
}

int recv_msg(const std::string& queue_name, std::string& msg) {

    std::string message;
    std::string div = ":";

    message = queue_name + div;
    std::string message_content = message.c_str();

    std::string header = create_header(RECVM, message_content.length());
    
    std::string complete_message = header + message_content; 
    send(client_fd, complete_message.c_str(), complete_message.length(), 0);


    char header_buffer[HEADER_SIZE];
    MsgType result;
    size_t response_length;
    int counter = 0;
    while (counter < HEADER_SIZE) {
        int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
        counter += header_bytes;
    }
    if (!parseHeader(header_buffer, result, response_length)) {
        printf("Invalid header from server \n");
        return 0;
    }

    if(result == SUCCESS) {
        std::vector<char> message_buf(response_length);
        int counter = 0;
        while (counter < response_length) {
            int response_bytes = recv(client_fd, message_buf.data() + counter, response_length - counter, 0);
            counter += response_bytes;
        }
        msg = std::string(message_buf.begin(), message_buf.end());
        return msg.length();
    }

    return 1;
}

int get_available_queues(std::string& queues){

    std::string header = create_header(LISTQ, 0);
    std::string complete_message = header; 
    send(client_fd, complete_message.c_str(), complete_message.length(), 0);
    
    char header_buffer[HEADER_SIZE];
    MsgType result;
    size_t response_length;
    int counter = 0;
    while (counter < HEADER_SIZE) {
        int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter,0 );
        counter += header_bytes;
    }
    if (!parseHeader(header_buffer, result, response_length)) {
        printf("Invalid header from server \n");
        return 0;
    }

    if(result == SUCCESS) {
        std::vector<char> message_buf(response_length);
        int counter = 0;
        while (counter < response_length) {
            int response_bytes = recv(client_fd, message_buf.data() + counter, response_length - counter, 0);
            counter += response_bytes;
        }
        queues = std::string(message_buf.begin(), message_buf.end());
        return 0;
    }

    return 1;
}