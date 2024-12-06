#include <unistd.h>
#include <cstring>
#include <cstdlib> 
#include <cstdlib>
#include <thread>
#include <sstream>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <vector> 
#include <queue>
#include <unordered_map>
#include <mutex>
#include "global.hpp"


std::atomic<int> client_id_counter{1};

struct Message{
    std::string text;
    time_t creation_time;
};

struct Queue {
    int holding_time;
    std::vector<int> queue_clients; 
    std::queue<Message> queue_messages;
};

class Serwer {
private:
    int server_fd;
    struct sockaddr_in server_addr;
    std::unordered_map<std::string, Queue> queues; // lista kolejek
    std::mutex queues_mutex;                        // mutex do synchronizacji dostępu do listy_kolejek

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
                    if (input.queue_name[0] != '\0' && input.holding_time > 0) {
                        int err = create_queue(input.queue_name, input.holding_time);
                        if(err == 0){
                            output.result = SUCCESS; 
                            subscribe(input.queue_name, client_id);
                            send(client_fd, &output, sizeof(output), 0);
                            printf("Client %d created queue %s with holding time of %d seconds.\n", client_id, input.queue_name, input.holding_time);
                        }else {
                            output.result = FAILURE; 
                            send(client_fd, &output, sizeof(output), 0);
                            printf("Queue creation failed, queue %s already exists.\n", input.queue_name);
                        }
                    } else {
                        output.result = FAILURE; 
                        send(client_fd, &output, sizeof(output), 0);
                        printf("Failed to create queue, invalid parameters.\n");
                    }
                    break;
                }
                case SUBSCRIBE: {
                    int err = subscribe(input.queue_name, client_id);
                    if(err == 0){
                        output.result = SUCCESS;
                        send(client_fd, &output, sizeof(output), 0);
                        printf("Client %d subscribed queue %s.\n", client_id, input.queue_name);
                    }
                    else if (err > 0){
                        output.result = FAILURE;
                        send(client_fd, &output, sizeof(output), 0);
                        if(err == 1){
                            printf("Failed to subscribe, queue %s does not exist.\n", input.queue_name);
                        }else{ 
                            printf("Client %d already subscribes queue %s.\n", client_id, input.queue_name);
                        }
                    }
                    break;
                }
                case UNSUBSCRIBE: {
                    int err = unsubscribe(input.queue_name, client_id);
                    if(err == 0){
                        output.result = SUCCESS;
                        send(client_fd, &output, sizeof(output), 0);
                        printf("Client %d unsubscribed queue %s.\n", client_id, input.queue_name);
                    }
                    else if (err > 0){
                        output.result = FAILURE;
                        send(client_fd, &output, sizeof(output), 0);
                        if(err == 1){
                            printf("Failed to unsubscribe, queue %s does not exist.\n", input.queue_name);
                        }else if (err == 2){ 
                            printf("Client %d does not subscribe queue %s.\n", client_id, input.queue_name);
                        }else 
                            printf("Failed to unsubscribe\n");
                    }
                    break;
                }
            }
        }
        close(client_fd);
    }

    int create_queue(const char * name, int holding_time){
        std::lock_guard<std::mutex> lock(queues_mutex); 
        if (queues.find(name) != queues.end()) {  // w unordered_map zwraca iterator wskazujacy na koniec jak nic nie znajdzie
            return 1;  
        }
        queues[name] = Queue{holding_time, {}, {}};
        return 0;
    }

    bool queue_exists(const char* name) {
        return queues.find(name) != queues.end();  
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
