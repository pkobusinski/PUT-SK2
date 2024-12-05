#include <iostream>
#include "msqCLI.hpp"

int main() {

    const char* server_ip = "127.0.0.1";
    int server_port = 8080;

    msqCLI client;
    int err = client.connect_to_server(server_ip, server_port);
    if (err != 0) {
        std::cerr << "Nie udało się połączyć z serwerem.\n";
        return 1;
    }
    
    client.create_queue("kolejka1", 60);
}