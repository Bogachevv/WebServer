#include "../socket.h"

#include <stdexcept>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>


base_socket::base_socket() {
    socket_fd = INVALID_SOCKET;
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == INVALID_SOCKET){
        throw std::runtime_error("Can't create socket. Error: " + std::to_string(WSAGetLastError()));
    }
}

base_socket::base_socket(int sock_fd) {

}