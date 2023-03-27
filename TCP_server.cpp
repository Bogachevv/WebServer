#include "TCP_server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>

TCP_server::TCP_server(const std::string& ip, int port) :
listener(ip, port, *this)
{

}

void TCP_server::on_accept(int fd, operation_type op_type) {
    if (op_type == operation_type::rd_av){
        int new_client_fd = accept(fd, nullptr, nullptr);
        clients.emplace_back(active_socket(new_client_fd, ...));
    }
    else throw std::runtime_error("Unexpected operation_type" + std::to_string((int)op_type));
}