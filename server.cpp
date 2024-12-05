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

#define BUFFER_SIZE 1024
std::atomic<int> client_id_counter{1};

enum CommandType {
    CREATE_QUEUE,
    UNKNOWN_COMMAND
};

CommandType parseCommand(const std::string& message) {
    if (message.find("CREATE_QUEUE") == 0) {
        return CREATE_QUEUE;
    }
    return UNKNOWN_COMMAND;
}

class Serwer {
private:
    int server_fd;
    struct sockaddr_in server_addr;

    void handleClient(int client_fd, int client_id) {
        char buffer[BUFFER_SIZE];
        while (true) {
            int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                printf("Client %d disconnected.\n", client_id);
                break;
            }
            
            std::string message(buffer);
            CommandType command = parseCommand(message);

            switch (command) {
                case CREATE_QUEUE: {
                    
                    std::istringstream stream(message);
                    std::string command, queue_name;
                    int holding_time;

                    stream >> command >> queue_name >> holding_time;

                    if (!queue_name.empty() && holding_time > 0) {
                        std::string response = "QUEUE_CREATED " + queue_name;
                        send(client_fd, response.c_str(), response.length(), 0);
                        //create_queue(queue_name, holding_time);
                        printf("Client %d created queue %s with holding time of %d seconds.\n", client_id, queue_name.c_str(), holding_time);
                    } else {
                        std::string response = "QUEUE_CREATION_FAILED";
                        send(client_fd, response.c_str(), response.length(), 0);
                        printf("Failed to create queue, invalid parameters.\n");
                    }
                    break;
                }
                case UNKNOWN_COMMAND: {
                    std::string response = "UNKNOWN_COMMAND";
                    send(client_fd, response.c_str(), response.length(), 0);
                    break;
                }
            }
        }
    close(client_fd);
    }

    int create_queue(std::string name, int holding_time){
        return 0;
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
