#include "response.h"

#include <utility>
#include <sstream>
#include <iterator>

response::response(int status_code, std::string status_msg) :
    status_code(status_code), status_msg(std::move(status_msg))
{
    HTTP_version = "HTTP/1.1";
}

response::response(int status_code, std::string status_msg, std::string HTTP_version)  :
        status_code(status_code),
        status_msg(std::move(status_msg)),
        HTTP_version(std::move(HTTP_version))
{

}

void response::set_body(std::vector<unsigned char> &&new_body) {
    body = std::vector<unsigned char>(new_body);
}

void response::set_body(const std::vector<unsigned char> &new_body) {
    body = new_body;
}

void response::add_header(const std::string &header) {
    headers.push_back(header);
}

void response::send(active_socket &socket) {
    std::stringstream resp;
    resp << HTTP_version << " " << status_code << " " <<  status_msg << "\n";
    for (auto &header: headers){
        resp << header << "\n";
    }
    resp << "\n";
    std::copy(body.begin(), body.end(), std::ostream_iterator<unsigned char>(resp, ""));
    socket << resp.str();
}