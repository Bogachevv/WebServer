#pragma once

#include <string>
#include <queue>
#include <vector>
#include <csignal>

#include "./IO_supervisor.h"


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
    ssize_t line_end;
    char *buf;
    size_t buf_size;

    void on_SIGIO(int fd, operation_type op_type, size_t size);

    void on_io_fin(size_t size);

    void on_rd_av();

    void on_wr_av();

public:
    active_socket();

    explicit active_socket(int sock_fd);

    void connect(const std::string &ip, int port);

    bool is_connected() const { return connected; }

    void shutdown(bool rd = true, bool wr = true);

    void write_buffer(const std::vector<char> &buffer);

    std::vector<char> read_buffer(size_t bytes_c);

    void write_line(const std::string &line);

    std::string read_line();

    bool str_available() const { return line_end > 0; }

    size_t rd_buf_size() const {return rd_buf.size(); }

    size_t wr_buf_size() const { return wr_buf.size(); }

    ~active_socket() noexcept override;
};


class listen_socket : public base_socket{
private:
    bool is_alive;

public:
    listen_socket(const std::string &ip, int port);

    void start_listening(int backlog);

    void stop_listening();

    ~listen_socket() override = default;
};
