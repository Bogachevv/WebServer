#include "IO_supervisor.h"

#include <aio.h>

int IO_supervisor_ptr::ref_c = 0;
IO_supervisor* IO_supervisor::obj_ptr = nullptr;

IO_supervisor_ptr::IO_supervisor_ptr(IO_supervisor *obj_ptr) {
    ++ref_c;
    this->obj_ptr = obj_ptr;
}

IO_supervisor_ptr::IO_supervisor_ptr(const IO_supervisor_ptr &other) {
    ++ref_c;
    this->obj_ptr = other.obj_ptr;
}

IO_supervisor_ptr::~IO_supervisor_ptr() {
    --ref_c;
    if (ref_c == 0){
        delete obj_ptr;
        obj_ptr = nullptr;
    }
}

IO_supervisor::IO_supervisor() : old_action() {
    struct sigaction act = {};
    act.sa_sigaction = SIGIO_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGIO, &act, &old_action);
}

IO_supervisor::~IO_supervisor() {
    sigaction(SIGIO, &old_action, nullptr);
}

IO_supervisor_ptr IO_supervisor::get() {
    if (IO_supervisor::obj_ptr == nullptr){
        IO_supervisor::obj_ptr = new IO_supervisor;
    }
    return IO_supervisor_ptr(IO_supervisor::obj_ptr);
}

void IO_supervisor::register_fd(int fd, const callback_t& callback) {
    callback_map.insert({fd, callback});
}

void IO_supervisor::unregister_fd(int fd) {
    if (callback_map.count(fd)) callback_map.erase(fd);
}

bool IO_supervisor::is_register(int fd) {
    return callback_map.count(fd) > 0;
}

void IO_supervisor::SIGIO_handler(int sig, siginfo_t *siginfo, void *ucontext) {
    int fd = siginfo->si_fd;
    operation_type op_type;
//    size_t size = 0;
    switch (siginfo->si_band) {
        case POLL_IN:
            op_type = operation_type::rd_av;
            break;
        case POLL_OUT:
            op_type = operation_type::wr_av;
            break;
        case SI_ASYNCIO:
            op_type = operation_type::io_fin;
//            size = aio_return64(((struct aiocb64*)siginfo->si_value.sival_ptr));
            break;
        default:
            op_type = operation_type::other;
            break;
    }
    auto &callback_map = IO_supervisor::obj_ptr->callback_map;
    if (callback_map.count(fd)){ //if fd in callback_map
        callback_map[fd](fd, op_type);
    }
}