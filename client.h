#pragma once

#include <queue>
#include <string>
#include <vector>
#include <stdexcept>

#include "./socket.h"

struct client_error : std::runtime_error {
    explicit client_error(const std::string& msg) : std::runtime_error(msg) {}
};

struct empty_buffer_error : client_error {
    explicit empty_buffer_error(const std::string& msg) : client_error(msg) {}
};

class client {
    active_socket socket;
    std::queue<unsigned char> in_buffer;
    std::queue<unsigned char> out_buffer;
    bool has_line;
    size_t line_end;

public:
    explicit client(active_socket sock);

    // move bytes from system socket buffer to in_buffer
    // return moved bytes count
    size_t read_package(size_t pack_size);

    // move bytes from out_buffer to system socket buffer
    // return moved bytes count
    size_t send_package(size_t pack_size);

    bool in_buffer_empty() const { return in_buffer.empty(); }

    bool out_buffer_empty() const { return out_buffer.empty(); }

    size_t bytes_in_out_buffer() const { return out_buffer.size(); }

    // pop char from in_buffer
    unsigned char pop_char();

    // pops chars from in_buffer into the result string until delimiter is found
    std::string pop_line();

    // pops bytes_c bytes from in_buffer into the result vector
    std::vector<unsigned char> pop_bytes(size_t bytes_c);

    void push_char(char ch);

    void push_line(const std::string &line);

    //void push_line(std:: string &&line); //implement if find std::move for iterators

    void push_bytes(const std::vector<unsigned char> &bytes);

    //void push_bytes(std::vector<unsigned char> &&bytes); //implement if find std::move for iterators

    active_socket &get_socket() { return socket; }

};
