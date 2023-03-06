#pragma once

#include <string>
#include <vector>

#include "./socket.h"

class response {
    std::string HTTP_version;
    int status_code;
    std::string status_msg;
    std::vector<std::string> headers;
    char *body;
    size_t body_size;

public:
    response(int status_code, std::string status_msg);

    response(int status_code, std::string status_msg, std::string HTTP_version);

    void set_text_body(const std::string &str);

    void copy_body(char *body_ptr, size_t size);

    void capture_body(char *body_ptr, size_t size);

    void add_header(const std::string &header);

    void send(active_socket &socket);

    ~response();
};

