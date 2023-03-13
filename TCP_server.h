#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <stdexcept>

#include "./socket.h"
#include "./client.h"

struct TCP_server_error : std::runtime_error{
    explicit TCP_server_error(const std::string &msg) : std::runtime_error(msg) {}
};

struct server_exists : TCP_server_error{
    explicit server_exists(const std::string &msg) : TCP_server_error(msg) {}
};

class TCP_server;

class TCP_server_ptr{
    static int ref_c;
    TCP_server *ptr;
public:
    explicit TCP_server_ptr(TCP_server *ptr);

    TCP_server_ptr(TCP_server_ptr &other);

    TCP_server_ptr &operator=(const TCP_server_ptr&) = delete;

    TCP_server *operator->() { return ptr; }

    ~TCP_server_ptr();

};

class TCP_server {
public:
    using pointer = TCP_server_ptr;
    using client_ptr = std::unique_ptr<client>;
    using client_process_callback = void(*)(client_ptr&);
    friend TCP_server_ptr;

private:
    static TCP_server *obj_ptr;

    std::unordered_map<int, client_ptr> clients_map;
    std::queue<int> rd_available;
    std::queue<int> process_available;

    listen_socket listener;

    void on_new_client();

    void main_loop();

    TCP_server(const std::string& ip, int port);

    ~TCP_server();

public:
    TCP_server(TCP_server&) = delete;
    TCP_server &operator=(const TCP_server&) = delete;

    static pointer make_server(const std::string& ip, int port);

    static pointer get_server();

    void start();
};
