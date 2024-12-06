#ifndef MSQCLI_HPP
#define MSQCLI_HPP

#include <string>

int connect_to_server(const char* ip, int port);
int disconnect();
int create_queue(const char* queue_name, int holding_time);
std::string get_available_queues();

// int subscribe(const char* queue_name);
// int unsubscribe(const char* queue_name);
// int send_msg(const char* queue_name, const char* msg, size_t msg_len);
// int recv_msg(const char* queue_name, const char* msg, size_t msg_len);
// dostepne kolejki 
#endif