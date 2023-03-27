#include "../socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdexcept>
#include <utility>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>

base_socket::base_socket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) throw std::runtime_error("Can't create socket");
}

base_socket::base_socket(int sock_fd) {
    if ((fcntl(sock_fd, F_GETFD) == -1) and (errno == EBADF))
        throw std::runtime_error("Can't create socket: " + std::to_string(sock_fd) + " is invalid file descriptor");
    socket_fd = sock_fd;
}

base_socket::~base_socket() noexcept {
    close(socket_fd);
}

active_socket::active_socket(std::weak_ptr<TCP_server> server_ptr)  :
    base_socket(),
    io_supervisor(IO_supervisor::get()),
    server(std::move(server_ptr))
{
    connected = false;
    auto callback = [&](int fd, operation_type op_type) {
        this->on_SIGIO(fd, op_type);
    };
    io_supervisor->register_fd(socket_fd, callback);
}

active_socket::active_socket(int sock_fd, std::weak_ptr<TCP_server> server_ptr) :
    base_socket(sock_fd),
    io_supervisor(IO_supervisor::get()),
    server(std::move(server_ptr))
{
    connected = false;
    auto callback = [&](int fd, operation_type op_type) {
        this->on_SIGIO(fd, op_type);
    };
    io_supervisor->register_fd(socket_fd, callback);
}

void active_socket::connect(const std::string &ip, int port) {
    if (connected) throw std::runtime_error("Socket already connected, use is_connected() to check");

    sockaddr_in sock_addr = {AF_INET, htons(port)};
    inet_pton(AF_INET, ip.c_str(), &(sock_addr.sin_addr));

    if (::connect(socket_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == -1)
        throw std::runtime_error("Can't connect socket");
}

void active_socket::shutdown(bool rd, bool wr) {
    if (not connected) return;
    if (rd and wr) ::shutdown(socket_fd, SHUT_RDWR);
    else if (rd) ::shutdown(socket_fd, SHUT_RD);
    else if (wr) ::shutdown(socket_fd, SHUT_WR);
}

void active_socket::write_buffer(const std::vector<char> &buffer) {
    for (auto &byte: buffer){
        wr_buf.push(byte);
    }
}

std::vector<char> active_socket::read_buffer(size_t bytes_c) {
    std::vector<char> res(std::min(bytes_c, rd_buf.size()));
    for (char & res_elm : res){
        res_elm = rd_buf.front();
        rd_buf.pop();
    }
    return res;
}

void active_socket::read_bytes() {
    size_t bytes_available = 0;
    if (ioctl(socket_fd, FIONREAD, &bytes_available)){
        throw std::runtime_error("ioctl error: " + std::string(strerror(errno)));
    }
    std::unique_ptr<char[]> buf(new char[bytes_available]);
    recv(socket_fd, buf.get(), bytes_available, 0);
    for (size_t i = 0; i < bytes_available; ++i){
        rd_buf.push(buf[bytes_available - i - 1]);
    }
}

void active_socket::write_bytes() {
    throw std::logic_error("Not implemented");
    //use aio to write bytes
}

void active_socket::on_SIGIO(int fd, operation_type op_type) {
    auto server_ptr = server.lock();
    if (server_ptr == nullptr){
        throw std::logic_error("Not implemented");
    }
    server_ptr->on_client_action(*this, op_type);
}

//TODO:
//  initialize server ptr
listen_socket::listen_socket(const std::string &ip, int port, const TCP_server& server_ref) :
    base_socket(),
    io_supervisor(IO_supervisor::get())
{
    auto callback = [&](int fd, operation_type op_type) {
        this->on_SIGIO(fd, op_type);
    };
    io_supervisor->register_fd(socket_fd, callback);

    sockaddr_in sock_addr = {AF_INET, htons(port)};
    inet_pton(AF_INET, ip.c_str(), &(sock_addr.sin_addr));

    if (bind(socket_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        throw std::runtime_error("Can't bind socket: " + std::string(strerror(errno)));
    }

    is_alive = false;
}

void listen_socket::start_listening(int backlog) {
    if (listen(socket_fd, backlog) == -1){
        throw std::runtime_error("Can't start listening socket" + std::string(std::strerror(errno)));
    }
    is_alive = true;
}

void listen_socket::on_SIGIO(int fd, operation_type op_type) {
    auto server_ptr = server.lock();
    if (server_ptr == nullptr){
        throw std::logic_error("Not implemented");
    }
    server_ptr->on_accept(fd, op_type);
}