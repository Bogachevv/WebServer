#include "response.h"

#include <utility>

response::response(int status_code, std::string status_msg) :
    status_code(status_code), status_msg(std::move(status_msg))
{
    HTTP_version = "HTTP/1.1";
    body = nullptr;
    body_size = 0;
}

response::response(int status_code, std::string status_msg, std::string HTTP_version)  :
        status_code(status_code),
        status_msg(std::move(status_msg)),
        HTTP_version(std::move(HTTP_version))
{
    body = nullptr;
    body_size = 0;
}

void response::set_text_body(const std::string &str) {
    this->body_size = str.length();
    this->body = new char[body_size];
    memcpy(this->body, str.c_str(), body_size);
}

void response::copy_body(char *body_ptr, size_t size) {
    this->body = new char[size];
    this->body_size = size;
    memcpy(this->body, body_ptr, size);
}

void response::capture_body(char *body_ptr, size_t size) {
    this->body = new char[size];
    this->body_size = size;
}

void response::add_header(const std::string &header) {
    headers.push_back(header);
}

void response::send(active_socket &socket) {
    std::string resp = HTTP_version + " " + std::to_string(status_code) + " " + status_msg + "\n";
    for (auto & header : headers){
        resp += header + "\n";
    }
    if (body_size) resp += "\n" + std::string(body) + "\n";
    socket << resp;
}

response::~response() {
    delete[] body;
    body = nullptr;
    body_size = 0;
}