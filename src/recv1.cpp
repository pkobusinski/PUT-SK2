#include "msqCLI.hpp"
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {
    connect_to_server("127.0.0.1", 8080);
    std::string msg;
    std::cout << create_queue("kolejka1", 10)<< std::endl;
    std::cout << recv_msg("kolejka1", msg) << std::endl;
    disconnect();

}
