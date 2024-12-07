#ifndef MSQCLI_HPP
#define MSQCLI_HPP

#include <string>
#include "global.hpp"

int connect_to_server(const char* ip, int port);
int disconnect();
int create_queue(const char* queue_name, int holding_time);

int subscribe(const char* queue_name);
int unsubscribe(const char* queue_name);

int send_msg(const char* queue_name, const char* msg);
int recv_msg(const char* queue_name, char* msg);
// std::string get_available_queues();

#endif