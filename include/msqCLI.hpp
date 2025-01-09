#ifndef MSQCLI_HPP
#define MSQCLI_HPP

#include <string>

int connect_to_server(const char* ip, int port);
int disconnect();
int create_queue(const std::string& queue_name, int holding_time);

int subscribe(const std::string& queue_name);
int unsubscribe(const std::string& queue_name);

int send_msg(const std::string& queue_name, const std::string& msg);
int recv_msg(const std::string& queue_name, std::string& msg);
int get_available_queues(std::string& queues);

#endif