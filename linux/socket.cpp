#include "../socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>


base_socket::base_socket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) throw std::runtime_error("Can't create socket");
}

base_socket::base_socket(socket_fd_t sock_fd) {
    socket_fd = sock_fd;
    //TODO:
    // check sock_fd
}

base_socket::~base_socket() noexcept {
    close(socket_fd);
}

listen_socket::listen_socket(const std::string &ip, int port, callback_t callback_ptr) : base_socket() {
    sockaddr_in sock_addr = {AF_INET, htons(port)};
    inet_pton(AF_INET, ip.c_str(), &(sock_addr.sin_addr));

    if (bind(socket_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        throw std::runtime_error("Can't bind socket");
    }

    this->callback_ptr = callback_ptr;
}

void listen_socket::start_listening(int backlog) {
    if (listen(socket_fd, backlog) == -1) throw std::runtime_error("Can't start listening socket");
    is_alive = true;
    listen_loop();
}

void listen_socket::stop_listening() {
    is_alive = false;
}

void listen_socket::set_callback(callback_t new_callback_ptr) {
    this->callback_ptr = new_callback_ptr;
}

void listen_socket::listen_loop() {
    while (is_alive){
        int client_fd = accept(socket_fd, NULL, NULL);
        active_socket client(client_fd);
        callback_ptr(client);
    }
}

active_socket::active_socket() : base_socket() {
    connected = false;
}

active_socket::active_socket(socket_fd_t sock_fd) : base_socket(sock_fd) {
    connected = false;
}

void active_socket::connect(const std::string &ip, int port) {
    if (connected) throw std::runtime_error("Socket already connected, use is_connected() to check");

    sockaddr_in sock_addr = {AF_INET, htons(port)};
    inet_pton(AF_INET, ip.c_str(), &(sock_addr.sin_addr));

    if (::connect(socket_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == -1)
        throw std::runtime_error("Can't connect socket");
}

bool active_socket::is_connected() const { return connected; }

void active_socket::shutdown(bool rd, bool wr) {
    if (not connected) return;
    if (rd and wr) ::shutdown(socket_fd, SHUT_RDWR);
    else if (rd) ::shutdown(socket_fd, SHUT_RD);
    else if (wr) ::shutdown(socket_fd, SHUT_WR);
}

void active_socket::send_msg(const char *buf, size_t len) {
    size_t bytes_sent = 0;
    while (bytes_sent < len){
        ssize_t sent = send(socket_fd, buf, len - bytes_sent, 0);
        if (sent < 0) throw std::runtime_error("Can't send message");
        bytes_sent += (size_t)sent;
        buf += (size_t)sent;
    }
}

void active_socket::receive_msg(char *buf, size_t len) {
    size_t bytes_received = 0;
    while (bytes_received < len){
        ssize_t received = recv(socket_fd, buf, len - bytes_received, 0);
        if (received < 0) throw std::runtime_error("Can't send message");
        else if (received == 0) break;
        bytes_received += (size_t)received;
        buf += (size_t)received;
    }
}

active_socket &active_socket::operator<<(const std::string &msg) {
    send_msg(msg.c_str(), msg.length());
    return *this;
}

active_socket &active_socket::operator>>(std::string &msg) {
    FILE *f = fdopen(dup(socket_fd), "r");
    //strange method to read c-string from socket
    //TODO:
    // rewrite it, using only low-level IO operations

    char *buf = NULL;
    size_t buf_len = 0;

    getline(&buf, &buf_len, f);

    msg = buf;
    free(buf);
    fclose(f);

    return *this;
}

active_socket &active_socket::operator<<(int msg) {
    uint32_t val = htonl(msg);
    send_msg((char*)(&val), sizeof(val));
    return *this;
}

active_socket &active_socket::operator>>(int &msg) {
    uint32_t val;
    receive_msg((char*)(&val), sizeof(val));
    msg = (int)ntohl(val);
    return *this;
}

active_socket::~active_socket() noexcept{
    shutdown(true, true);
}