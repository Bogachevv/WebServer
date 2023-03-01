#include "WSA.h"

#include <stdexcept>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

WSA *WSA::object_ptr = nullptr;
int WSA_ptr::ref_c = 0;

WSA::WSA() : wsaData() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        throw std::runtime_error("Can't startup WSA");
    }
}

WSA::~WSA() {
    WSACleanup();
}

WSA_ptr WSA::getWSA() {
    if (WSA::object_ptr == nullptr){
        WSA::object_ptr = new WSA();
    }
    return WSA_ptr(WSA::object_ptr);
}

WSA_ptr::WSA_ptr(WSA *wsa_ptr) {
    ++ref_c;
    ptr = wsa_ptr;
}

WSA_ptr::WSA_ptr(const WSA_ptr &) {
    //TODO: implement
}

WSA_ptr::~WSA_ptr() {
    --ref_c;
    if (ref_c == 0){
        delete ptr;
    }
}