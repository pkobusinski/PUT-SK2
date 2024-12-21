#include "msqCLI.hpp"

static int client_fd = -1;

int connect_to_server(const char* ip, int port) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        return 1;
    }

    return 0;
}

int disconnect() {
    if (client_fd >= 0) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        client_fd = -1;
        return 0;
    }else   
        return 1;
}

int create_queue(const char* queue_name, int holding_time) {
    if (queue_name[0] != '\0' && holding_time > 0) {
        fbs action;
        fsb answer;
        action.command = CREATE_QUEUE;
        strcpy(action.queue_name, queue_name); 
        action.holding_time = holding_time;
        
        send(client_fd, &action, sizeof(action), 0);
        int bytes_received = read(client_fd, &answer, sizeof(answer));
        if (bytes_received > 0) {
            if (answer.result == SUCCESS) {
                return 0;
            }
        }
    }
    
    return 1;
}

int subscribe(const char* queue_name) {
    fbs action;
    fsb answer;
    action.command = SUBSCRIBE;
    strcpy(action.queue_name, queue_name); 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

int unsubscribe(const char* queue_name) {
    fbs action;
    fsb answer;
    action.command = UNSUBSCRIBE;
    strcpy(action.queue_name, queue_name); 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return 0;
        }
    }
    return 1;
}

int send_msg(const char* queue_name, const char* msg) {
    fbs action;
    fsb answer;
    action.command = SEND;
    strcpy(action.queue_name, queue_name); 
    strcpy(action.message, msg);

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            return answer.msg_len;
        }
    }
    return 1;
}

int recv_msg(const char* queue_name, char* msg) {
    fbs action;
    fsb answer;
    action.command = RECV;
    strcpy(action.queue_name, queue_name); 
 

    send(client_fd, &action, sizeof(action), 0);
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            strcpy(msg, answer.message);
            return answer.msg_len;
        }
    }
    return 1;
}

int get_available_queues(char * queues){
    fbs action;
    fsb answer;
    action.command = LIST_QUEUES;
    send(client_fd, &action, sizeof(action), 0);
    
    int bytes_received = read(client_fd, &answer, sizeof(answer));
    if (bytes_received > 0) {
        if (answer.result == SUCCESS) {
            strcpy(queues, answer.message);
            return 0;
        }
    }
    return 1;

}