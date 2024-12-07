#include <iostream>
#include "msqCLI.hpp"

int main() {

    // const char* server_ip = "127.0.0.1";
    // int server_port = 8080;

    connect_to_server("127.0.0.1", 8080);
    create_queue("kolejka1", 5);
    unsubscribe("kolejka1");
    subscribe("kolejka1");
    send_msg("kolejka1", "hello");
    send_msg("kolejka1", "abcd");
    send_msg("kolejka1", "efghijk");
    char buf[255];
    int cnt = recv_msg("kolejka1", buf );
    printf("buf: %s, cnt: %d \n", buf, cnt);
    cnt = recv_msg("kolejka1", buf );
    printf("buf: %s, cnt: %d \n", buf, cnt);

    while(true){}
    disconnect();
}
