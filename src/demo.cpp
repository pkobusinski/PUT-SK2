#include "msqCLI.hpp"
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {

    if(fork() == 0){
        connect_to_server("127.0.0.1", 8080);
        std::cout << create_queue("kolejka1", 5) << std::endl;
        std::cout << create_queue("kolejka2", 10)<< std::endl;
        std::cout << subscribe("kolejka1") << std::endl;
        std::cout << unsubscribe("kolejka2") << std::endl;
        std::cout << send_msg("kolejka1", "test") << std::endl;
        std::string msg;
        std::cout << recv_msg("kolejka1", msg) << std::endl;
        std::cout << send_msg("kolejka1", "last") << std::endl;
        std::cout << msg << std::endl;
        disconnect();
    }else{
        std::string buf;
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        subscribe("kolejka");
        subscribe("kolejka1");
        int cnt = recv_msg("kolejka1", buf);
        std::cout << "buf: " << buf << ", cnt: " << cnt << std::endl;
        send_msg("kolejka1", "123");
        std::cout << get_available_queues(buf) << std::endl;
        std::cout << buf << std::endl;
        cnt = recv_msg("kolejka1", buf);
        std::cout << "buf: " << buf << ", cnt: " << cnt << std::endl;
        
        disconnect();
    }
}
