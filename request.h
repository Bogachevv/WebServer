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
    std::string host;
    std::string HTTP_version;
    std::vector<std::string> headers;
    std::string body;

public:
    request(request_method method, std::string path, std::string HTTP_version);

    explicit request(const std::string &);

    void add_header(const std::string &header);

    const std::string &get_path() const;
    const std::string &get_host() const;
    const std::string &get_HTTP_version() const;
    request_method get_request_method() const;

    void send(active_socket &socket);

    const std::string &operator[](size_t pos);

    std::string str() const;

    friend std::ostream &operator<<(std::ostream &stream, const request &req);
};


request accept_request(active_socket &socket);