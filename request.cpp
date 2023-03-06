#include "request.h"

#include <utility>
#include <stdexcept>

request::request(request_method method, std::string path, std::string HTTP_version) :
    method(method), path(std::move(path)), HTTP_version(std::move(HTTP_version))
{

}

request::request(const std::string &str) {
    size_t beg_pos = 0;
    size_t end_pos = str.find(' ', beg_pos);
    std::string method_str = str.substr(beg_pos, end_pos - beg_pos);
    if (method_str == "GET") method = request_method::GET;
    else if (method_str == "POST") method = request_method::POST;
    else if (method_str == "HEAD") method = request_method::HEAD;

    beg_pos = end_pos + 1;
    end_pos = str.find(' ', beg_pos);
    path = str.substr(beg_pos, end_pos - beg_pos);

    beg_pos = end_pos + 1;
    end_pos = str.find(' ', beg_pos);
    HTTP_version = str.substr(beg_pos, end_pos - beg_pos);
}

void request::add_header(const std::string &header) {
    headers.push_back(header);
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

request_method request::get_request_method() const {
    return method;
}

const std::string &request::operator[](size_t pos) {
    return headers.at(pos);
}

void request::send(active_socket &socket) {
    throw std::logic_error("request::send is not implemented yet");
}

std::string request::str() const {
    std::string str;
    switch (method) {
        case request_method::GET:
            str = "GET";
            break;
        case request_method::POST:
            str = "POST";
            break;
        case request_method::HEAD:
            str = "HEAD";
            break;
    }
    str += " " + path + " " + HTTP_version + "\n";
    for (auto &header: headers){
        str += header + "\n";
    }
    str += "\n";
    if (not body.empty()){
        str += body + "\n";
    }

    return str;
}

std::ostream &operator<<(std::ostream &stream, const request &req) {
    stream << req.str();
    return stream;
}

bool is_empty(const std::string &str){
    return (str.empty() or (str[0] == '\r'));
}

request accept_request(active_socket &socket) {
    std::string line = socket.read_line('\n');
    request res(line);
    int elc = 1;

    while (elc){
        line = socket.read_line('\n');
        if (is_empty(line)) --elc;
        else{
            res.add_header(line);
        }
    }

    return res;
}


