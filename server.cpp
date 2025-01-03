#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cstdlib> 
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "global.hpp"


std::atomic<int> client_id_counter{1};       // obsługa synchronizacji przydzielania client_id


class Serwer {
private:
    int server_fd;
    struct sockaddr_in server_addr;
    std::unordered_map<std::string, Queue> queues;          //lista kolejek 
    std::mutex queues_mutex;                                //obsługa synchronizacji dostępu do listy kolejek 
    std::condition_variable queue_condition;                //obsługa synchronizacji odczytywania wiadomości z kolejki (czekanie dopóki nie pojawi się nowa wiadomość)                        

    void handleClient(int client_fd, int client_id) {
        
        fbs input;
        fsb output;
        while (true) {
            int bytes_read = read(client_fd, &input, sizeof(input));
            if (bytes_read <= 0) {
                printf("Client %d disconnected.\n", client_id);
                break;
            }

            switch (input.command) {
                case CREATE_QUEUE: {
                    int err = create_queue(input.queue_name, input.holding_time);
                    output.result = (err == 0) ? SUCCESS : FAILURE;
                    if(err == 0){
                        subscribe(input.queue_name, client_id);
                    }
                    send(client_fd, &output, sizeof(output), 0);
                    switch (err) {
                        case 0:
                            printf("Client %d created queue %s with holding time of %d seconds.\n", client_id, input.queue_name, input.holding_time);
                            break;
                        default:
                            printf("Queue creation failed, queue %s already exists.\n", input.queue_name);
                            break;
                    }
                    break;
                }
                case SUBSCRIBE: {
                    int err = subscribe(input.queue_name, client_id);
                    output.result = (err == 0) ? SUCCESS : FAILURE;
                    send(client_fd, &output, sizeof(output), 0);
                    switch (err) {
                        case 0:
                            printf("Client %d subscribed queue %s.\n", client_id, input.queue_name);
                            break;
                        case 1:
                            printf("Failed to subscribe, queue %s does not exist.\n", input.queue_name);
                            break;
                        case 2:
                            printf("Failed to subscribe, Client %d already subscribes queue %s.\n", client_id, input.queue_name);
                            break;
                    }
                    break;
                }
                case UNSUBSCRIBE: {
                    int err = unsubscribe(input.queue_name, client_id);
                    output.result = (err == 0) ? SUCCESS : FAILURE;
                    send(client_fd, &output, sizeof(output), 0);
                    switch (err) {
                        case 0:
                            printf("Client %d unsubscribed queue %s.\n", client_id, input.queue_name);
                            break;
                        case 1:
                            printf("Failed to unsubscribe, queue %s does not exist.\n", input.queue_name);
                            break;
                        case 2:
                            printf("Failed to unsubscribe, Client %d does not subscribe queue %s.\n", client_id, input.queue_name);
                            break;
                        default:
                            printf("Failed to unsubscribe\n");
                            break;
                    }
                    break;
                }
                case SEND: {
                    int err = send_message(input.queue_name, input.message, client_id);
                    output.result = (err == 0) ? SUCCESS : FAILURE;
                    if(err == 0){
                        output.msg_len = strlen(input.message);
                    }
                    send(client_fd, &output, sizeof(output), 0);
                    switch (err) {
                        case 0:
                            printf("Client %d sent msg %s to queue %s.\n", client_id, input.message, input.queue_name);
                            break;
                        case 1:
                            printf("Failed to send msg, queue %s does not exist.\n", input.queue_name);
                            break;
                        case 2:
                            printf("Failed to send msg, Client %d does not subscribe queue %s.\n", client_id, input.queue_name);
                            break;
                    }
                    break;
                }
                case RECV: {
                    int err = recv_message(input.queue_name, output.message, client_id);
                    output.result = (err == 0) ? SUCCESS : FAILURE;
                    if(err == 0){
                        output.msg_len = strlen(output.message);
                    }
                    send(client_fd, &output, sizeof(output), 0);
                    switch (err) {
                        case 0:
                            printf("Client %d received msg %s from queue %s.\n", client_id, output.message, input.queue_name);
                            printf("Message removed from queue %s.\n", input.queue_name);
                            break;
                        case 1:
                            printf("Failed to receive msg, queue %s does not exist.\n", input.queue_name);
                            break;
                        case 2:
                            printf("Failed to receive msg, Client %d does not subscribe queue %s.\n", client_id, input.queue_name);
                            break;
                    }
                    break;
                }
                case LIST_QUEUES: {
                    get_queue_names(output.message);
                    output.msg_len = strlen(output.message);
                    output.result = (output.msg_len != 0) ? SUCCESS : FAILURE;
                    send(client_fd, &output, sizeof(output), 0);
                    break;
                }
            }

        }
        
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    int create_queue(const char * name, int holding_time){
        std::lock_guard<std::mutex> lock(queues_mutex); 
        if (queues.find(name) != queues.end()) {            // w unordered_map .find() zwraca iterator wskazujacy na koniec jak nic nie znajdzie
            return 1;  
        }
        queues[name] = Queue{holding_time, {}, {}};
        return 0;
    }

    bool queue_exists(const char* name) { 
        for (const auto& pair : queues) {
            if (pair.first == name) {  
                return true;
            }
        }
        return false;
    }

    bool is_subscribed(const char* name, int client_id) {
        
        if(!queue_exists(name))
            return false;
        auto it = queues.find(name);
        
        auto& clients = it->second.queue_clients;
        for (int i = 0; i < clients.size(); ++i) {
            if (clients[i] == client_id) {
                return true;  
            }
        }
        return false;  
    }

    int subscribe(const char* name, int client_id) {
        std::lock_guard<std::mutex> lock(queues_mutex);
        if (!queue_exists(name)) {
            return 1;  
        }

        if (is_subscribed(name, client_id)) {
            return 2;  
        }

        auto& clients = queues[name].queue_clients;
        clients.push_back(client_id);
        return 0;
    }

    int unsubscribe(const char* name, int client_id) {
        std::lock_guard<std::mutex> lock(queues_mutex);
        if (!queue_exists(name)) {
            return 1;  
        }

        if (!is_subscribed(name, client_id)) {
            return 2;  
        }

        auto& clients = queues[name].queue_clients;
        for (int i = 0; i < clients.size(); ++i) {              
            if (clients[i] == client_id) {
                clients[i] = clients.back();
                clients.pop_back();
                return 0;  
            }
        }

        return 3;
    }
    
    int send_message(const char* queue_name, const char* message_text, int client_id) {
        std::lock_guard<std::mutex> lock(queues_mutex);

        if (!queue_exists(queue_name)) {
            return 1;  
        }

        if (!is_subscribed(queue_name, client_id)) {
            return 2;  
        }

        
        Message msg;
        msg.creation_time = time(NULL);
        msg.is_read = false;
        strcpy(msg.text, message_text); 
        queues[queue_name].queue_messages.push(msg);

        queue_condition.notify_one();

        return 0;  
    }


    int recv_message(const char* queue_name, char* message_text, int client_id) {
        std::unique_lock<std::mutex> lock(queues_mutex);    

        if (!queue_exists(queue_name)) {
            return 1;  
        }

        if (!is_subscribed(queue_name, client_id)) {
            return 2;  
        }

        auto& queue = queues[queue_name];
        
        while (queue.queue_messages.empty()) {
            queue_condition.wait(lock);  
        }

        Message& msg = queue.queue_messages.front();
        strcpy(message_text, msg.text);  
        queue.queue_messages.pop(); 

        return 0;  
    }

    void get_queue_names(char* result) {
        std::lock_guard<std::mutex> lock(queues_mutex);  

        result[0] = '\0';   
        bool first = true;  

        for (const auto& pair : queues) {
            const std::string& queue_name = pair.first;

            if (!first) {
                strcat(result, ",");
            }
            first = false;
            strcat(result, queue_name.c_str());
        }
    }

    void remove_expired_messages() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));  

            std::lock_guard<std::mutex> lock(queues_mutex);
            for (auto& queue_entry : queues) {
                auto& queue = queue_entry.second;
                while (!queue.queue_messages.empty()) {
                    Message& msg = queue.queue_messages.front();
                    time_t current_time = time(NULL);
                    if (current_time - msg.creation_time >= queue.holding_time) {
                        queue.queue_messages.pop();  
                        printf("Message removed from queue %s.\n", queue_entry.first.c_str());  
                    }
                }
            }
        }
    }

public:
    Serwer(const std::string& addr, int port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        server_addr.sin_family = AF_INET;
        if (inet_aton( addr.c_str(), &server_addr.sin_addr) <= 0) {
            perror("Invalid address");
            exit(EXIT_FAILURE);
        }
        server_addr.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, SOMAXCONN) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
        printf("Server is listening on %s:%hu\n", addr.c_str(), port);
        std::thread(&Serwer::remove_expired_messages, this).detach();
    }

    void run() {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }

            int client_id = client_id_counter++;
            printf("Client %d connected from %s:%d \n", client_id, inet_ntoa(client_addr.sin_addr), 
                    ntohs(client_addr.sin_port));

            std::thread(&Serwer::handleClient, this, client_fd, client_id).detach();
        }
    }

    ~Serwer() {
        std::lock_guard<std::mutex> lock(queues_mutex); 
        queues.clear();

        close(server_fd);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP ADRESS> <PORT>\n", argv[0]);
        return 1;
    }

    std::string address = argv[1];
    long port = strtol(argv[2], 0, 10);
    if(port < 0 || port > 65656){
        printf("Invalid port number");
        return 1;
    }

    Serwer serwer(address, port);
    serwer.run();
    return 0;
}
