#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdexcept>

#include "../socket.h"


base_socket::base_socket() : wsaPtr(WSA::getWSA()) {
    socket_fd = INVALID_SOCKET;
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == INVALID_SOCKET){
        throw std::runtime_error("Can't create socket. Error: " + std::to_string(WSAGetLastError()));
    }
}

base_socket::base_socket(socket_fd_t sock_fd) : wsaPtr(WSA::getWSA()) {
    socket_fd = sock_fd;
}

base_socket::~base_socket() noexcept {
    closesocket(socket_fd);
}

active_socket::active_socket() : base_socket() {
    connected = false;
}

active_socket::active_socket(socket_fd_t sock_fd) : base_socket(sock_fd) {
    connected = false;
}

void active_socket::connect(const std::string &ip, int port) {
    if (connected) throw std::runtime_error("Socket already connected, use is_connected() to check");

    sockaddr_in sock_addr = {};
    sock_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &(sock_addr.sin_addr));
    sock_addr.sin_port = htons(port);

    if (::connect(socket_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
        throw std::runtime_error("Can't connect socket");
}

bool active_socket::is_connected() const { return connected; }

void active_socket::shutdown(bool rd, bool wr) {
    if (not connected) return;
    if (rd and wr) ::shutdown(socket_fd, SD_BOTH);
    else if (rd) ::shutdown(socket_fd, SD_RECEIVE);
    else if (wr) ::shutdown(socket_fd, SD_SEND);
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
    buf[0] = 0;
    while (bytes_received < len){
        ssize_t received = recv(socket_fd, buf, len - bytes_received, 0);
        if (received < 0) throw std::runtime_error("Can't receive message");
        else if (received == 0) break;
        bytes_received += (size_t)received;
        buf += (size_t)received;
    }
}

std::string active_socket::read_line(char delimiter) {
    std::string msg;
    while (true){
        char ch = 0;
        ssize_t received = recv(socket_fd, &ch, 1, 0);
        if (received < 0) throw std::runtime_error("Can't receive message");
        msg.push_back(ch);
        if ((received == 0) or (ch == delimiter) or (ch == 0)) break;
    }

    return msg;
}

active_socket &active_socket::operator<<(const std::string &msg) {
    send_msg(msg.c_str(), msg.length());
    return *this;
}

active_socket &active_socket::operator>>(std::string &msg) {
    while (true){
        char ch = 0;
        ssize_t received = recv(socket_fd, &ch, 1, 0);
        if (received < 0) throw std::runtime_error("Can't receive message");
        if ((received == 0) or (ch == 0)) break;
        msg.push_back(ch);
    }

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

listen_socket::listen_socket(const std::string &ip, int port, callback_t callback_ptr) : base_socket() {
    sockaddr_in sock_addr = {};
    sock_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(sock_addr.sin_addr));
    sock_addr.sin_port = htons(port);

    if (bind(socket_fd, (sockaddr*)(&sock_addr), sizeof(sock_addr)) == SOCKET_ERROR){
        closesocket(socket_fd);
        throw std::runtime_error("Can't bind socket");
    }

    this->is_alive = false;
    this->callback_ptr = callback_ptr;
}

void listen_socket::start_listening(int backlog) {
    if (listen(socket_fd, backlog) == SOCKET_ERROR){
        throw std::runtime_error("Can't listen socket");
    }
    is_alive = true;
    listen_loop();
}

void listen_socket::stop_listening() {
    is_alive = false;
}

void listen_socket::set_callback(callback_t new_callback_ptr) {
    this->callback_ptr = new_callback_ptr;
}

//TODO:
//  rewrite for parallel work with clients
//  A single-threaded approach for working with a client's needs may only be necessary
//  for the purpose of improving debugging convenience
void listen_socket::listen_loop() {
    while (is_alive){
        socket_fd_t client_fd = accept(socket_fd, NULL, NULL);
        active_socket client(client_fd);
        callback_ptr(client);
    }
}