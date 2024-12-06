#include <iostream>
#include "msqCLI.hpp"

int main() {

    // const char* server_ip = "127.0.0.1";
    // int server_port = 8080;

    connect_to_server("127.0.0.1", 8080);
    std::cout << get_available_queues() << std::endl; 
    create_queue("kolejka1", 60);
    std::cout << get_available_queues() << std::endl;

    while(true){}
    disconnect();
}
