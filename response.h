#pragma once

#include <string>
#include <vector>

#include "./socket.h"

class response {
    std::string HTTP_version;
    int status_code;
    std::string status_msg;
    std::vector<std::string> headers;
    std::string body;

public:
    response(int status_code, std::string status_msg);

    void set_body(std::string body);

    void add_header(const std::string &header);

    void send(active_socket &socket);

};

