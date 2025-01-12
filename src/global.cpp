#include "global.hpp"
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

std::string create_header(int command_type, int message_length) {
    std::ostringstream oss;
    oss << std::setw(10) << std::setfill('0') << message_length;
    std::string padded_length = oss.str();

    std::string header;
    std::string div = ":";
    header = std::to_string(command_type) + div + padded_length + "|";
    return header;
}

bool parseHeader(const std::string& header, MsgType& command, int& message_length) {    
    int div_pos =  header.find(':');
    int type = std::stoi(header.substr(0, div_pos));
    command = (MsgType) type;
    int header_end_pos = header.find("|");
    message_length = std::stoi(header.substr(div_pos+1, header_end_pos -1));
    return true;
}

void string_procent_encode(std::string& s){
    std::string encoded; 
    for (char c : s) {
        if (c == ':') {
            encoded += "%3A";
        } else {
            encoded += c;
        }
    }
    s = encoded;
}

void string_procent_decode(std::string& s){
    std::string decoded; 
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '%' && s[i+1] == '3' && s[i+2] == 'A') {
            decoded += ':';
            i += 2;
        } else {
            decoded += s[i];
        }
    }
    s = decoded;
}