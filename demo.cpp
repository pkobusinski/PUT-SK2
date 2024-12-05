#include <iostream>
#include "msqCLI.hpp"

int main() {

    const char* server_ip = "127.0.0.1";
    int server_port = 8080;

    int err = connect_to_server(server_ip, server_port);
    if (err != 0) {
        printf("Failed to connect to server.\n");
        return 1;
    }
    
    create_queue("kolejka1", 60);
    

    disconnect();
}
