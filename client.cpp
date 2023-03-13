#include "client.h"

#include <utility>

client::client(active_socket sock) : socket(std::move(sock)) {
    has_line = false;
    line_end = 0;
}

unsigned char client::pop_char() {
    if (in_buffer_empty()) throw empty_buffer_error("in_buffer is empty");
    if (has_line) {
        --line_end;
        if (line_end == 0) has_line = false;
    }
    unsigned char res = in_buffer.front();
    in_buffer.pop();
    return res;
}

std::string client::pop_line() {
    if (not has_line) throw client_error("Client has not yet read the whole line");

    std::string res(line_end, 0);
    for (size_t i = 0; i < line_end; ++i){
        res[i] = (char)in_buffer.front();
        in_buffer.pop();
    }

    has_line = false;
    return res;
}

std::vector<unsigned char> client::pop_bytes(size_t bytes_c) {
    if (bytes_in_out_buffer() < bytes_c)
        throw client_error("Client has not yet read " + std::to_string(bytes_c) + " bytes");
    if (has_line) {
        if (line_end > bytes_c) line_end -= bytes_c;
        else has_line = false;
    }

    std::vector<unsigned char> res(bytes_c, 0);
    for (size_t i = 0; i < bytes_c; ++i){
        res[i] = (char)in_buffer.front();
        in_buffer.pop();
    }

    return res;
}

void client::push_char(char ch) {
    out_buffer.push(ch);
}

void client::push_line(const std::string &line) {
    for (auto ch: line){
        out_buffer.push(ch);
    }
}

void client::push_bytes(const std::vector<unsigned char> &bytes) {
    for (auto byte: bytes){
        out_buffer.push(byte);
    }
}

//TODO:
//  1) Implement read_package (with line_checking), send_package