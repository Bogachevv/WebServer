#include <utility>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <csignal>
#include <wait.h>

#include "../CGI.h"

int change_fd(int src, int copy){
    if (copy == src) return 0;
    int state = dup2(src, copy);
    close(src);
    return state;
}

CGI::CGI(std::string path, const std::vector<std::string> &args, const std::vector<std::string> &env) :
    script_path(std::move(path))
{
    if (pipe(pipes) == -1){
        throw CGI_pipe_error("Can't create pipe");
    }

    pid = fork();
    if (pid == -1)
        throw CGI_fork_error("Can't create CGI: fork error (" + std::string(std::strerror(errno)) + ")");
    if (pid > 0) { // father

    }
    else{ // son
        change_fd(pipes[0], 0);
        change_fd(pipes[1], 1);

        // which better?
        auto args_cptr = new decltype(args[0].c_str())[args.size()];
        auto env_cptr  = new typename std::string::const_pointer[env.size()];

        for (size_t i = 0; i < args.size(); ++i){
            args_cptr[i] = args[i].c_str();
        }
        for (size_t i = 0; i < env.size(); ++i){
            env_cptr[i] = env[i].c_str();
        }

        execvpe(script_path.c_str(), (char* const*)args_cptr, (char* const*)env_cptr);
        throw CGI_fork_error("Can't start script: " + std::string(std::strerror(errno)));
    }
}

std::string CGI::process(const std::string &input) {
    write(pipes[1], input.c_str(), input.size());
    char buf[128];
    std::string out;
    ssize_t rd_cnt;
    do{
        rd_cnt = read(pipes[0], buf, sizeof(buf));
        if (rd_cnt == -1)
            throw CGI_pipe_error("Can't read from pipe: " + std::string(std::strerror(errno)));
        std::copy(buf, buf+rd_cnt, out.end());
    } while (rd_cnt == sizeof(buf));

    return out;
}

CGI::~CGI() {
    close(pipes[0]);
    close(pipes[1]);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, WNOHANG);
}