#pragma once

#include "IO_supervisor.h"
#include "socket.h"

#include <queue>
#include <vector>
#include <memory>

enum class mode{
    rd, wr, process, wait
};


class TCP_server {
listen_socket listener;

std::vector<std::shared_ptr<active_socket>> clients;
std::vector<std::shared_ptr<active_socket>> process_available;

public:
    TCP_server(const std::string& ip, int port);

    TCP_server(const TCP_server&) = delete;

    TCP_server &operator=(const TCP_server&) = delete;

//    TCP_server(TCP_server&& rhs);

//    TCP_server &operator=(TCP_server&& rhs) noexcept;

    void on_client_action(active_socket &client, operation_type op_type);

    void on_accept(int fd, operation_type op_type);
};
