#pragma once

#include <string>
#include <vector>

#include "./socket.h"

class response {
    std::string HTTP_version;
    int status_code;
    std::string status_msg;
    std::vector<std::string> headers;
    std::vector<unsigned char> body;

public:
    response(int status_code, std::string status_msg);

    response(int status_code, std::string status_msg, std::string HTTP_version);

    void set_body(const std::vector<unsigned char> &new_body);

    void set_body(std::vector<unsigned char> &&new_body);

    void add_header(const std::string &header);

    void send(active_socket &socket);

    ~response() = default;
};

