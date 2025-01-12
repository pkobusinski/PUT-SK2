#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>  
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <memory>
#include <csignal> 
#include "global.hpp"


class Serwer {
private:
    int server_fd;  
    struct sockaddr_in server_addr;
    std::unordered_map<std::string, Queue> queues;          
    std::mutex queues_mutex;                                // mutex dla ogolnego dostepu do kolejek 
    std::condition_variable queue_condition;                // zmienna warunkowa obslugujaca oczekiwanie na pojawienie sie wiadomsci w kolejce   
    int client_id_counter = 1;
    bool running = true; 
    std::vector<std::thread> threads; 

    std::string create_response(MsgType response_type, std::string msg){
        std::string header = create_header(response_type, msg.length());
        if(msg.empty()){
            return header;
        }
        return header + msg;
    }

    void handleClient(int client_fd, int client_id) {                       // obsluga klienta 
        
        char header_buffer[HEADER_SIZE];
        std::string response;
        
        while (running) {
           int counter = 0;
            while (counter < HEADER_SIZE) {                                 // zapewnienie odczytania calego naglowka 
                int header_bytes = recv(client_fd, header_buffer + counter, HEADER_SIZE - counter, 0);
                if (header_bytes <= 0 ) {
                    printf("Client %d disconnected.\n", client_id);
                    shutdown(client_fd, SHUT_RDWR);
                    close(client_fd);
                    return;
                }
                counter += header_bytes;
            }

            MsgType result;
            MsgType command;
            int message_length;
            if (!parseHeader(header_buffer, command, message_length)) {
                printf("Invalid header from client %d\n", client_id);
                return ;
            }
            
            counter = 0;
            std::vector<char> message_buf(message_length);
            
            if (message_length > 0) {                                       
                while (counter < message_length) {                          // zapewnienie odczytania calej wiadomosci 
                    int message_bytes = recv(client_fd, message_buf.data() + counter, message_length - counter, 0);
                    if (message_bytes <= 0) {
                        printf("Client %d disconnected.\n", client_id);
                        shutdown(client_fd, SHUT_RDWR);
                        close(client_fd);
                        return;
                    }
                    counter += message_bytes;
                }
            }
            
            std::string message(message_buf.begin(), message_buf.end());
            
            switch (command) {                      // obsluga poszczegolnych funkcji biblioteki 
                case CREAT: {
                    int div_pos = message.find(":");
                    
                    std::string queue_name = message.substr(0, div_pos);
                    int holding_time = std::stoi(message.substr(div_pos + 1));
                    string_procent_decode(queue_name);
                    
                    int err = create_queue(queue_name, holding_time);
                    result = (err == 0) ? SUCCESS : FAILURE;

                    response = create_response(result, "");
                    send(client_fd, response.c_str(), response.length(), 0);

                    if (err == 0) {
                        printf("Client %d created queue %s with holding time of %d seconds.\n", client_id, queue_name.c_str(), holding_time);
                        subscribe(queue_name, client_id);
                    } else {
                        printf("Queue creation failed, queue %s already exists.\n", queue_name.c_str());
                    }

                    break;
                }
                case SUBSC: {
                    int div_pos = message.find(':');
                    std::string queue_name = message.substr(0, div_pos);
                    string_procent_decode(queue_name);
                    int err = subscribe(queue_name, client_id);
                    result = (err == 0) ? SUCCESS : FAILURE;
                    response = create_response(result, "");
                    send(client_fd, response.c_str(), response.length(), 0);

                    switch (err) {
                        case 0:
                            printf("Client %d subscribed queue %s.\n", client_id, queue_name.c_str());
                            break;
                        case 1:
                            printf("Failed to subscribe, queue %s does not exist.\n", queue_name.c_str());
                            break;
                        case 2:
                            printf("Failed to subscribe, Client %d already subscribes queue %s.\n", client_id, queue_name.c_str());
                            break;
                        default:
                            break; 
                    }
                    break;
                }
                case UNSUB: {
                    int div_pos = message.find(':');
                    std::string queue_name = message.substr(0, div_pos);
                    string_procent_decode(queue_name);
                    int err = unsubscribe(queue_name, client_id);
                    result = (err == 0) ? SUCCESS : FAILURE;
                    response = create_response(result, "");
                    send(client_fd, response.c_str(), response.length(), 0);

                    switch (err) {
                        case 0:
                            printf("Client %d unsubscribed queue %s.\n", client_id, queue_name.c_str());
                            break;
                        case 1:
                            printf("Failed to unsubscribe, queue %s does not exist.\n", queue_name.c_str());
                            break;
                        case 2:
                            printf("Failed to unsubscribe, Client %d does not subscribe queue %s.\n", client_id, queue_name.c_str());
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case SENDM: {
                    int div_pos = message.find(':');
                    std::string queue_name = message.substr(0, div_pos);
                    std::string msg = message.substr(div_pos + 1);
                    string_procent_decode(queue_name);
                    string_procent_decode(msg);
                    int err = send_message(queue_name, msg, client_id);
                    result = (err == 0) ? SUCCESS : FAILURE;
                
                    if(result == SUCCESS)
                        response = create_response(result, std::to_string(msg.length()));
                    else
                        response = create_response(result, "");

                    send(client_fd, response.c_str(), response.length(), 0);

                    switch (err) {
                        case 0:
                            printf("Client %d sent msg %s to queue %s.\n", client_id, msg.c_str(), queue_name.c_str());
                            break;
                        case 1:
                            printf("Failed to send msg, queue %s does not exist.\n", queue_name.c_str());
                            break;
                        case 2:
                            printf("Failed to send msg, Client %d does not subscribe queue %s.\n", client_id, queue_name.c_str());
                            break;
                        default:
                            break; 
                    }
                    break;
                }
                case RECVM: {
                    int div_pos = message.find(':');
                    std::string queue_name = message.substr(0, div_pos);
                    std::string msg; 
                    
                    string_procent_decode(queue_name);
                    int err = recv_message(queue_name, msg, client_id);
                    result = (err == 0) ? SUCCESS : FAILURE;
                    std::string msg_encoded = msg; 
                    string_procent_encode(msg_encoded);

                    if(result == SUCCESS)
                        response = create_response(result, msg_encoded);
                    else
                        response = create_response(result, "");

                    send(client_fd, response.c_str(), response.length(), 0);

                    switch (err) {
                        case 0:
                            printf("Client %d received msg %s from queue %s.\n", client_id, msg.c_str(), queue_name.c_str());
                            printf("Message removed from queue %s.\n", queue_name.c_str());
                            break;
                        case 1:
                            printf("Failed to receive msg, queue %s does not exist.\n", queue_name.c_str());
                            break;
                        case 2:
                            printf("Failed to receive msg, Client %d does not subscribe queue %s.\n", client_id, queue_name.c_str());
                            break;
                        default: 
                            break;
                    }
                    break;
                }
                case LISTQ: {
                    std::string list;
                    result = SUCCESS;
                    get_queue_names(list);
                    response = create_response(result, list);
                    send(client_fd, response.c_str(), response.length(), 0);
                    printf("Client %d requested list of queues.\n", client_id);
                    break;
                }
                default:{
                    response = FAILURE;
                    response = create_response(result, "");
                    send(client_fd, response.c_str(), response.length(), 0);
                    break;
                }
            }

            

        }

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        
    }
    
    int create_queue(std::string &name, int holding_time){
        std::lock_guard<std::mutex> lock(queues_mutex); 
        if (queues.count(name)) {            
            return 1;  
        }
        queues[name] = Queue{holding_time, {}, {}};
        return 0;
    }

    bool is_subscribed(std::string& name, int client_id) {
        
        if(!queues.count(name))
            return false;
        auto it = queues.find(name);
        
        auto& clients = it->second.queue_clients;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i] == client_id) {
                return true;  
            }
        }
        return false;  
    }

    int subscribe(std::string& name, int client_id) {

        std::lock_guard<std::mutex> lock(queues_mutex);
        if (!queues.count(name)) {
            return 1;  
        }

        if (is_subscribed(name, client_id)) {
            return 2;  
        }

        auto& clients = queues[name].queue_clients;
        clients.push_back(client_id);
        return 0;
    }

    int unsubscribe(std::string& name, int client_id) {
        std::lock_guard<std::mutex> lock(queues_mutex);
        if (!queues.count(name)) {
            return 1;  
        }

        if (!is_subscribed(name, client_id)) {
            return 2;  
        }

        auto& clients = queues[name].queue_clients;
        for (size_t i = 0; i < clients.size(); ++i) {              
            if (clients[i] == client_id) {
                clients[i] = clients.back();
                clients.pop_back();
                return 0;  
            }
        }

        return 3;
    }
    
    int send_message(std::string& queue_name, std::string& message_text, int client_id) {
        std::lock_guard<std::mutex> lock(queues_mutex);

        if (!queues.count(queue_name)) {
            return 1;  
        }

        if (!is_subscribed(queue_name, client_id)) {
            return 2;  
        }

        
        Message msg;
        msg.creation_time = time(NULL);
        msg.is_read = false;
        msg.text = message_text; 
        queues[queue_name].queue_messages.push(msg);

        queue_condition.notify_one();

        return 0;  
    }

    int recv_message(std::string& queue_name, std::string& message_text, int client_id) {
        std::unique_lock<std::mutex> lock(queues_mutex);

        if (!queues.count(queue_name)) {
            return 1;
        }

        if (!is_subscribed(queue_name, client_id)) {
            return 2;
        }

        auto& queue = queues[queue_name];
        queue_condition.wait(lock, [&queue, this] {
            return !queue.queue_messages.empty() || !running;
        });

        if (!running) {
            return 3;
        }

        Message& msg = queue.queue_messages.front();
        message_text = msg.text;
        queue.queue_messages.pop();

        return 0;
    }

    void get_queue_names(std::string& result) {
        std::lock_guard<std::mutex> lock(queues_mutex);  

        result.clear();   
        bool first = true; 
        if(queues.empty()){
            result = "";
            return;
        }
            
        for (const auto& pair : queues) {
            const std::string& queue_name = pair.first;

            if (!first) {
                result += ",";
            }
            first = false;
            result += queue_name;
        }
    }

    void remove_expired_messages() {
        while (running) {
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
                    } else {
                        break;  
                    }
                }
            }
        }
    }

public:
    Serwer(int port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
    }
    
    ~Serwer(){
        stop();
    }

    void stop(){
        running = false; 
        queue_condition.notify_all();

        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        
    }


    void run() {
        std::cout << "Server is listening on port" << ":" << ntohs(server_addr.sin_port) << std::endl;

        while (running) {
            int client_fd;
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, 
                &client_addr_len)) < 0) {
                perror("Accept failed");
                continue;
            }

            threads.emplace_back(&Serwer::remove_expired_messages, this);

            int client_id = client_id_counter++; 
            std::cout << "Client " << client_id << " connected from " 
                      << inet_ntoa(client_addr.sin_addr) << ":" 
                      << ntohs(client_addr.sin_port) << std::endl;
                      
            threads.emplace_back(&Serwer::handleClient, this, client_fd, client_id);
            
        }
        std::cout << "Server closed" << std::endl;
    }
};

Serwer * global_server = nullptr; 

void signal_handler(int signum) {
    if (global_server) {
        std::cout << "\nServer closed" << std::endl;
        global_server->stop();
    }
    std::exit(signum);
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    Serwer server(port);

    global_server = &server; 

    std::signal(SIGINT, signal_handler);
    server.run();
    return 0;
}