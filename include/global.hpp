#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <vector>
#include <queue>
#include <string> 

#define HEADER_SIZE 13              // ustalona stala dlugosc naglowka dla wszystkich wiadomosci 

enum MsgType {                      // typy przesylanych wiadomosci 
    CREAT,
    SUBSC,
    UNSUB,
    SENDM,
    RECVM,
    LISTQ, 
    SUCCESS,
    FAILURE
};


struct Message {                    // struktura wiadomosci przetrzymywanych w kolejkach  
    std::string text;  
    time_t creation_time;
    bool is_read;  
};

struct Queue {                      // struktura kolejki przetrzymujacej wiadomosci 
    int holding_time;
    std::vector<int> queue_clients;
    std::queue<Message> queue_messages;
};



std::string create_header(int command_type, int message_length);                                //tworzenie naglowka dla wiadomosc wysylanych przez server 
bool parseHeader(const std::string& header, MsgType& command, int& message_length);             //przetwarzanie informacji z naglowka wiadomosci 
void string_procent_decode(std::string& s);                                                        //umozliwienie uzycia znaku ':' w nazwach kolejek i wiadomosciach 
void string_procent_encode(std::string& s);     

#endif