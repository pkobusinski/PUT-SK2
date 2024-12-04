#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>

#include <msqCLI.hpp>

#define PORT 12345
#define BUFFER_SIZE 1024

std::mutex queue_mutex;
std::atomic<int> client_id_counter(0);

struct Message {
    std::string text;
    time_t creation_time;
    int hold_time;
};

class Serwer {
private:
    int server_fd;
    struct sockaddr_in server_addr;
    int server_addrlen = sizeof(server_addr);
    std::map<int, int> clients;
    std::mutex clients_mutex;

    void handle_client(int client_fd, int client_id) {
        char buffer[BUFFER_SIZE];
        std::cout << "Client " << client_id << " connected.\n";

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                std::cout << "Client " << client_id << " disconnected.\n";
                break;
            }

            std::cout << "Received from client " << client_id << ": " << buffer << std::endl;

            // Echo back the message
            send(client_fd, buffer, strlen(buffer), 0);
        }

        close(client_fd);
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(client_id);
    }

public:
    Serwer() {
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Server socket creation failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, SOMAXCONN) < 0) {
            perror("Listen");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server is listening at: " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port) << "\n";
    }

    void run() {
        while (true) {
            std::cout << "Waiting for a new client...\n";
            int client_fd = accept(server_fd, (struct sockaddr *)&server_addr, (socklen_t *)&server_addrlen);
            if (client_fd < 0) {
                perror("Accept");
                continue;
            }

            int client_id = ++client_id_counter;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients[client_id] = client_fd;
            }

            std::thread(&Serwer::handle_client, this, client_fd, client_id).detach();
        }
    }
};

int main() {
    Serwer serwer;
    serwer.run();
    return 0;
}
