#include "../socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <aio.h>

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

active_socket::active_socket() : base_socket(), io_supervisor(IO_supervisor::get()) {
    connected = false;
    line_end = -1;
    buf_size = 512;
    buf = new char[buf_size];
    auto callback = [&](int fd, operation_type op_type, size_t size) {
        this->on_SIGIO(fd, op_type, size);
    };
    io_supervisor->register_fd(socket_fd, callback);
}

active_socket::active_socket(int sock_fd) : base_socket(sock_fd), io_supervisor(IO_supervisor::get()) {
    connected = false;
    line_end = -1;
    buf_size = 512;
    buf = new char[buf_size];
    auto callback = [&](int fd, operation_type op_type, size_t size) {
        this->on_SIGIO(fd, op_type, size);
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

void active_socket::write_line(const std::string &line) {
    for (auto &byte: line){
        wr_buf.push(byte);
    }
}

std::string active_socket::read_line() {
    if (not str_available()) throw std::runtime_error("Line is not available");
    std::string res(line_end, '\0');

    for (ssize_t i = 0; i < line_end; ++i){
        res[i] = rd_buf.front();
        rd_buf.pop();
    }

    return res;
}

void active_socket::on_SIGIO(int fd, operation_type op_type, size_t size) {
    if (op_type == operation_type::rd_av) on_rd_av();
    else if (op_type == operation_type::wr_av) on_wr_av();
    else if (op_type == operation_type::io_fin) on_io_fin(size);
    else throw std::runtime_error("Unexpected op_type");
}

void active_socket::on_rd_av() {
    struct sigevent se{SIGEV_SIGNAL, SIGIO};
    struct aiocb64 io_req{socket_fd, LIO_READ, 0, buf, buf_size, se};
    aio_read64(&io_req);
}

void active_socket::on_wr_av() {
    struct sigevent se{SIGEV_SIGNAL, SIGIO};
    struct aiocb64 io_req{socket_fd, LIO_WRITE, 0, buf, buf_size, se};
    aio_read64(&io_req);
}

active_socket::~active_socket() noexcept {
    delete[] buf;
    // TODO:
    //  should i close sock_fd??
}