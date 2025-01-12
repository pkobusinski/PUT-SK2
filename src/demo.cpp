#include "msqCLI.hpp"
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {

    if(fork() == 0){
        connect_to_server("127.0.0.1", 8080);
        std::cout << create_queue("kol:ejka1", 5) << std::endl;
        std::cout << create_queue("kolejka2", 10)<< std::endl;
        std::cout << subscribe("kol:ejka1") << std::endl;
        std::cout << unsubscribe("kolejka2") << std::endl;
        std::cout << send_msg("kol:ejka1", "te::st") << std::endl;
        std::string msg;
        std::cout << recv_msg("kol:ejka1", msg) << std::endl;
        std::cout << send_msg("kol:ejka1", "last") << std::endl;
        std::cout << msg << std::endl;
        subscribe("kolejka7");
        disconnect();
    }else if (fork() ==0){
            std::string buf;
            connect_to_server("127.0.0.1", 8080);
            std::cout << get_available_queues(buf) << std::endl;
            std::cout << buf << std::endl;
            create_queue("kolejka0", 5);
            subscribe("kolejka");
            subscribe("kolejka1");
            send_msg("kolejka1", "123");
            int cnt = recv_msg("kolejka8", buf);
            std::cout << "buf: " << buf << ", cnt: " << cnt << std::endl;
            disconnect();
        }else{
        std::string buf;
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        create_queue("kolejka7", 5);
        subscribe("kolejka");
        unsubscribe("kolejka7");
        send_msg("kolejka1", "123");
        std::string buf2;
        std::cout << get_available_queues(buf2) << std::endl;
        std::cout << buf2 << std::endl;
        int cnt = recv_msg("kolejka1", buf);
        std::cout << "buf: " << buf << ", cnt: " << cnt << std::endl;
        subscribe("kolejka8");
        disconnect();
    }
}
