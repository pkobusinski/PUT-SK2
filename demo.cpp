#include "msqCLI.hpp"

int main() {

    if(fork() == 0){
        char buf[255];
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        subscribe("kolejka1");
        send_msg("kolejka1", "abc");
        send_msg("kolejka1", "defg");
        int cnt = recv_msg("kolejka1", buf );
        printf("buf: %s, cnt: %d \n", buf, cnt);
        send_msg("kolejka1", "hijkl");
        cnt = recv_msg("kolejka1", buf );
        printf("buf: %s, cnt: %d \n", buf, cnt);
        
        disconnect();
        

    }else{
        char buf[255];
        connect_to_server("127.0.0.1", 8080);
        create_queue("kolejka1", 5);
        subscribe("kolejka");
        get_available_queues(buf);
        printf("%s\n", buf);
        subscribe("kolejka1");
        int cnt = recv_msg("kolejka1", buf);
        printf("buf: %s, cnt: %d \n", buf, cnt);
        send_msg("kolejka1", "123");
        cnt = recv_msg("kolejka1", buf);
        printf("buf: %s, cnt: %d \n", buf, cnt);
        
        while(true){}

    }
    
}
