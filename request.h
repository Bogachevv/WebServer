#pragma once

#include <string>
#include <vector>

#include "./socket.h"

enum class request_method{
    GET, POST, HEAD
};


class request {
    request_method method;
    std::string path;
    std::string HTTP_version;
    std::vector<std::string> headers;

public:
    request(request_method method, std::string path);

    void add_header(const std::string &header);

    bool add_line(std::string line);

    const std::string &get_path() const;
    const std::string &get_host() const;
    const std::string &get_HTTP_version() const;

    void send(active_socket &socket);

    const std::string &operator[](size_t pos);
};

