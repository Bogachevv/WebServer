#pragma once

#include <sys/types.h>
#include <csignal>
#include <unistd.h>
#include <unordered_map>
#include <functional>

class IO_supervisor;

class IO_supervisor_ptr{
    static int ref_c;

    IO_supervisor *obj_ptr;
public:
    explicit IO_supervisor_ptr(IO_supervisor *obj_ptr);

    IO_supervisor_ptr(const IO_supervisor_ptr &other);

    IO_supervisor_ptr& operator=(const IO_supervisor_ptr&) = delete;

    IO_supervisor *operator->() { return obj_ptr; }

    ~IO_supervisor_ptr();
};

enum class operation_type{
    rd_av, wr_av, io_fin, waiting, other
};

class IO_supervisor {
public:
    typedef std::function<void(int, operation_type, size_t)> callback_t;

    friend IO_supervisor_ptr;
private:
    static IO_supervisor *obj_ptr;

    struct sigaction old_action;

    std::unordered_map<int, callback_t> callback_map;

    static void SIGIO_handler(int sig, siginfo_t *siginfo, void *ucontext);

    IO_supervisor();

    ~IO_supervisor();
public:
    IO_supervisor(const IO_supervisor&) = delete;

    IO_supervisor& operator=(const IO_supervisor&) = delete;

    static IO_supervisor_ptr get();

    // TODO: add fd to SIGIO users
    void register_fd(int fd, const callback_t& callback);

    void unregister_fd(int fd);

    bool is_register(int fd);

};
