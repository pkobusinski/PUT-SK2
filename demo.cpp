#include <iostream>
#include "msqCLI.hpp"

int main() {

    if(fork() == 0){
        char buf[255];
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        unsubscribe("kolejka1");
        subscribe("kolejka1");
        send_msg("kolejka1", "abc");
        send_msg("kolejka1", "defg");
        int cnt = recv_msg("kolejka1", buf );
        printf("c1, buf: %s, cnt: %d \n", buf, cnt);
        send_msg("kolejka1", "hijkl");
        cnt = recv_msg("kolejka1", buf );
        printf("c1, buf: %s, cnt: %d \n", buf, cnt);

    while(true){}
    disconnect();
    }else{
        char buf[255];
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        subscribe("kolejka1");
        int cnt = recv_msg("kolejka1", buf);
        printf("c2, buf: %s, cnt: %d \n", buf, cnt);
        send_msg("kolejka1", "123");
        cnt = recv_msg("kolejka1", buf);
        printf("c2, buf: %s, cnt: %d \n", buf, cnt);

    }
    
}
