#include "response.h"

#include <utility>

response::response(int status_code, std::string status_msg) :
    status_code(status_code), status_msg(std::move(status_msg))
{

}

void response::set_body(std::string new_body) {
    this->body = std::string(std::move(new_body));
}

void response::add_header(const std::string &header) {
    headers.push_back(header);
}

void response::send(active_socket &socket) {
    std::string resp = HTTP_version + " " + std::to_string(status_code) + " " + status_msg + "\n";
    for (auto & header : headers){
        resp += header + "\n";
    }
    if (not body.empty()) resp += "\n" + body + "\n";
    socket << resp;
}