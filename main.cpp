#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include "socket.h"
#include "request.h"
#include "response.h"
#include "CGI.h"

#define PORT 8000

void send404(active_socket &client){
    std::string error_msg = "<h1>File not found</h1>";

    response resp(404, "Not Found");
    resp.add_header("Content-Type: text/html; charset=UTF-8");
    resp.add_header("Content-Length: " + std::to_string(error_msg.length()));

    resp.set_body({error_msg.begin(), error_msg.end()});
    resp.send(client);
}

enum class file_type{
    text, image, exec, other
};

class file_open_error : std::runtime_error {
    std::string path;
public:
    file_open_error(const std::string &msg, std::string path) : std::runtime_error(msg), path(std::move(path)) {}
};

std::string get_file_extension(const std::string &path){
    size_t dot = path.find_last_of('.');
    std::string ext = path.substr(dot+1);
    return ext;
}

file_type get_file_type(const std::string &path){
    static const std::unordered_map<std::string, file_type> file_types = {
            {"htm", file_type::text},
            {"html", file_type::text},
            {"css", file_type::text},
            {"js", file_type::text},
            {"png", file_type::image},
            {"jpg", file_type::image},
            {"jpeg", file_type::image},
            {"gif", file_type::image},
            {"webp", file_type::image},
            {"sh", file_type::exec},
            {"mjs", file_type::exec},
            {"py", file_type::exec}
    };
    std::string f_type = get_file_extension(path);

    if (file_types.count(f_type) > 0) return file_types.at(f_type);
    return file_type::other;
}

size_t add_bin_body(response &resp, const std::string &path){
    std::ifstream file(path, std::ios::binary);
    if (file.bad()) throw file_open_error("Can't open file", path);

    //read file to char vector
    std::vector<unsigned char> body(std::istreambuf_iterator<char>(file), {});
    size_t body_size = body.size();

    resp.set_body(std::move(body));
    return body_size;
}

size_t add_text_body(response &resp, const std::string &path){
    std::ifstream file(path);
    if (file.bad()) throw file_open_error("Can't open file", path);

    size_t i;
    std::string body;
    for (i = 0; !file.eof(); ++i){
        char ch;
        file.get(ch);
        body.push_back(ch);
    }
    std::cout << body;

    resp.set_body({body.begin(), body.end()});
    return i;
}

size_t add_from_CGI(response &resp, const std::string &path){
    //TODO: get env vars from query string
    std::cout << "running CGI" << std::endl;
//    std::string tmp_path("./script.py");
    CGI cgi(path, {}, {});
    std::cout << "processing CGI" << std::endl;
    auto res = cgi.process();
    resp.set_body({res.begin(), res.end()});
    std::cout << "result of CGI: " << res << std::endl;
    return res.length();
}

void add_body(response &resp, const std::string &path){
    std::cout << "add body" << std::endl;
    file_type f_type = get_file_type(path);
    size_t body_size;
    switch (f_type) {
        case file_type::text:
            body_size = add_text_body(resp, path);
            resp.add_header("Content-Type: text/" + get_file_extension(path) +"; charset=UTF-8");
            resp.add_header("Content-Length: " + std::to_string(body_size));
            break;
        case file_type::image:
            body_size = add_bin_body(resp, path);
            resp.add_header("Content-Type: image/" + get_file_extension(path));
            resp.add_header("Content-Length: " + std::to_string(body_size));
            break;
        case file_type::exec:
            body_size = add_from_CGI(resp, path);
            resp.add_header("Content-Type: text/html; charset=UTF-8");
            resp.add_header("Content-Length: " + std::to_string(body_size));
            break;
        case file_type::other:
            return;
    }
}

void callback(active_socket &client){
    request req = accept_request(client);
    std::cout << req;
    std::cout << std::endl << std::endl;

    if (req.get_request_method() == request_method::GET){
        std::string path = req.get_path();
        if (path.length() <= 1){
//            path = R"(..\www\index.html)";
            path = "../www/index.html";
        } else{
//            path = R"(..\www\)" + path.substr(1, path.length());
            path = "../www/" + path.substr(1, path.length());
        }

        response resp(200, "OK");
        try{
            add_body(resp, path);
        }
        catch (file_open_error&){
            send404(client);
            return;
        }
        resp.send(client);
    }
}

void server(){
    listen_socket server("172.26.126.209", PORT);
    std::cout << "Start server" << std::endl;
    server.set_callback(&callback);
    server.start_listening(16);
}

int main(int argc, char **argv) {
    server();
    return 0;
}
