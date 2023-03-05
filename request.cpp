#include "request.h"

#include <utility>
#include <stdexcept>

request::request(request_method method, std::string path) :
    method(method), path(std::move(path))
{
    HTTP_version = std::string("HTTP/1.1");
}

void request::add_header(const std::string &header) {
    headers.push_back(header);
}

bool request::add_line(std::string line) {
    if (line == "\r\n") return false;

}

const std::string &request::get_path() const {
    return path;
}

const std::string &request::get_host() const {
    return host;
}

const std::string &request::get_HTTP_version() const {
    return HTTP_version;
}

const std::string &request::operator[](size_t pos) {
    return headers.at(pos);
}

void request::send(active_socket &socket) {
    throw std::logic_error("request::send is not implemented yet");
}
