#pragma once

#include "IO_supervisor.h"
#include "socket.h"

#include <queue>
#include <vector>

enum class mode{
    rd, wr, process, wait
};

struct client{
    int fd;
    mode m;
};

class TCP_server {
listen_socket listener;
IO_supervisor_ptr supervisor_ptr;

std::queue<int> fd_rd_available;
std::queue<int> fd_wr_available;
std::queue<int> fd_process_available;

std::vector<client> clients;

void on_accept(int fd, operation_type op_type);

public:
    TCP_server(const std::string& ip, int port);

    TCP_server(const TCP_server&) = delete;

    TCP_server &operator=(const TCP_server&) = delete;

    TCP_server(TCP_server&& rhs);

    TCP_server &operator=(TCP_server&& rhs) noexcept;


};
