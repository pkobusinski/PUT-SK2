#ifndef MSQCLI_HPP
#define MSQCLI_HPP

#include <string>

int connect_to_server(const char* ip, int port);
int disconnect();
int create_queue(const char* queue_name, int holding_time);

#endif