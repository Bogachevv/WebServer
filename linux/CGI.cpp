#include <utility>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <csignal>
#include <wait.h>
#include <fcntl.h>
#include <fstream>

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
    tmp_path = std::string("Process_") + std::to_string(pid) + std::string(".tmp");
    if (pipe(pipes) == -1){
        throw CGI_pipe_error("Can't create pipe");
    }

    pid = fork();
    if (pid == -1)
        throw CGI_fork_error("Can't create CGI: fork error (" + std::string(std::strerror(errno)) + ")");
    if (pid > 0) { // father
        close(pipes[0]);
    }
    else{ // son
        close(pipes[1]);
        change_fd(pipes[0], 0);
        int tmp_file = open(tmp_path.c_str(),
                            O_CREAT | O_TRUNC | O_WRONLY);
        change_fd(tmp_file, 1);

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
    int status;
    while (waitpid(pid, &status, 0) != pid) {}

    if (WIFEXITED(status) and (WEXITSTATUS(status) == 0)){
        std::ifstream tmp_file(tmp_path);
        if (tmp_file.bad()) throw std::runtime_error("Can't open tmp file");
        std::string res = {std::istreambuf_iterator<char>(tmp_file), std::istreambuf_iterator<char>()};

        tmp_file.close();
        std::remove(tmp_path.c_str());

        return res;
    }
    throw std::runtime_error("CGI script error");
}

CGI::~CGI() = default;