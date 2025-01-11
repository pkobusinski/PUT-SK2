#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <vector>
#include <queue>
#include <string> 

#define HEADER_SIZE 13

enum MsgType {
    CREAT,
    SUBSC,
    UNSUB,
    SENDM,
    RECVM,
    LISTQ, 
    SUCCESS,
    FAILURE
};


struct Message {
    std::string text;  
    time_t creation_time;
    bool is_read;  
};

struct Queue {
    int holding_time;
    std::vector<int> queue_clients;
    std::queue<Message> queue_messages;
};



std::string create_header(int command_type, size_t message_length); 
bool parseHeader(const std::string& header, MsgType& command, size_t& message_length);
void string_procent_decode(std::string& s);
void string_procent_encode(std::string& s);

#endif