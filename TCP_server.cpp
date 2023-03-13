#include "TCP_server.h"

#include <utility>

int TCP_server_ptr::ref_c = 0;
TCP_server *TCP_server::obj_ptr = nullptr;

TCP_server_ptr::TCP_server_ptr(TCP_server *ptr) {
    this->ptr = ptr;
    ++ref_c;
}

TCP_server_ptr::TCP_server_ptr(TCP_server_ptr &other) {
    this->ptr = other.ptr;
    ++ref_c;
}

TCP_server_ptr::~TCP_server_ptr() {
    --ref_c;
    if (ref_c == 0){
        delete ptr;
        ptr = nullptr;
    }
}

TCP_server::pointer TCP_server::make_server(const std::string& ip, int port) {
    if (TCP_server::obj_ptr != nullptr) throw server_exists("TCP server already exists");
    TCP_server::obj_ptr = new TCP_server(ip, port);

    TCP_server_ptr ptr(obj_ptr);
    return ptr;
}

TCP_server::pointer TCP_server::get_server() {
    TCP_server_ptr ptr(obj_ptr);
    return ptr;
}


//TODO:
// 1) Write async version of sockets (lister_socket, active_socket)
// 2) Implement constructor (set SIGIO handler)
// 3) Implement destructor (set default (IGNORE) reaction to SIGIO)
// 4) Implement main loop and on_new_client
// 5) Implement async file IO(for each client request)