#pragma once

#include <string>
#include <vector>
#include <stdexcept>

struct CGI_error : std::runtime_error{
    explicit CGI_error(const char* msg) : std::runtime_error(msg) {}
    explicit CGI_error(const std::string &msg) : std::runtime_error(msg) {}
};

struct CGI_fork_error : CGI_error{
    explicit CGI_fork_error(const char* msg) : CGI_error(msg){}
    explicit CGI_fork_error(const std::string &msg) : CGI_error(msg){}
};

struct CGI_pipe_error : CGI_error {
    explicit CGI_pipe_error(const char* msg) : CGI_error(msg) {}
    explicit CGI_pipe_error(const std::string &msg) : CGI_error(msg){}
};

class CGI {
    std::string script_path;
    pid_t pid;
    // pipes[0] -- rd
    // pipes[1] -- wr
    int pipes[2];

public:
    CGI(const std::string &path, const std::vector<std::string> &args, const std::vector<std::string> &env);

    std::string process();

    ~CGI();
};
