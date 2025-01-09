#include "global.hpp"
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

std::string create_header(int command_type, size_t message_length) {
    std::ostringstream oss;
    oss << std::setw(10) << std::setfill('0') << message_length;
    std::string padded_length = oss.str();

    std::string header;
    std::string div = ":";
    // CMD:00LEN|msg1:msg2:msg3
    header = std::to_string(command_type) + div + padded_length + "|";
    return header;
}

bool parseHeader(const std::string& header, MsgType& command, size_t& message_length) {    
    int div_pos =  header.find(':');
    int type = std::stoi(header.substr(0, div_pos));
    command = (MsgType) type;
    int header_end_pos = header.find("|");
    message_length = std::stoi(header.substr(div_pos+1, header_end_pos -1));
    return true;
}

