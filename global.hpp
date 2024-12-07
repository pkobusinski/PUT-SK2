#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <string>

enum CommandType {
    CREATE_QUEUE,
    SUBSCRIBE,
    UNSUBSCRIBE,
    SEND,
    RECV,
    LIST_QUEUES
};

enum ResultType {
    SUCCESS,
    FAILURE
};

struct fbs {    // struktra do komunikacji sieciowej biblioteka -> serwer 
    CommandType command;
    char queue_name[255];
    char message[255]; 
    int holding_time;
};

struct fsb {  // struktra do komunikacji sieciowej serwer -> biblioteka 
    ResultType result;
    char message[255];
    size_t msg_len;
    
};


#endif