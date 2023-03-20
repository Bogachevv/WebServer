#include "TCP_server.h"

TCP_server::TCP_server(const std::string& ip, int port) :
listener(ip, port), supervisor_ptr(IO_supervisor::get())
{
    auto lmbd = [&](int fd, operation_type op_type) { this->on_accept(fd, op_type); };
    supervisor_ptr->register_fd(listener.get_fd(), lmbd);
}