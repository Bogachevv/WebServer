#pragma once

#include <string>
#include <queue>
#include <vector>
#include <csignal>

#include "./IO_supervisor.h"
#include "./TCP_server.h"


class base_socket {
    base_socket(const base_socket &other);
    const base_socket &operator=(const base_socket &other);
protected:
    int socket_fd;

    virtual ~base_socket() noexcept;
public:
    base_socket();

    int get_fd() const { return socket_fd; }

    explicit base_socket(int sock_fd);
};


class active_socket : public base_socket{
    bool connected;
    IO_supervisor_ptr io_supervisor;

    std::queue<char> rd_buf, wr_buf;
    std::weak_ptr<TCP_server> server;

    void on_SIGIO(int fd, operation_type op_type);

public:
    explicit active_socket(std::weak_ptr<TCP_server> server_ptr);

    active_socket(int sock_fd, std::weak_ptr<TCP_server> server_ptr);

    void connect(const std::string &ip, int port);

    bool is_connected() const { return connected; }

    void shutdown(bool rd = true, bool wr = true);

    void write_buffer(const std::vector<char> &buffer);

    std::vector<char> read_buffer(size_t bytes_c);

    size_t rd_buf_size() const {return rd_buf.size(); }

    size_t wr_buf_size() const { return wr_buf.size(); }

    void read_bytes();

    void write_bytes();

    ~active_socket() noexcept override = default;
};


class listen_socket : public base_socket{
    bool is_alive;
    std::weak_ptr<TCP_server> server;
    IO_supervisor_ptr io_supervisor;

    void on_SIGIO(int fd, operation_type op_type);
public:
    listen_socket(const std::string &ip, int port, const TCP_server& server_ref);

    void start_listening(int backlog);

    ~listen_socket() override = default;
};
