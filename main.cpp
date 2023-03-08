#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "socket.h"
#include "request.h"
#include "response.h"

#define PORT 127

void send404(active_socket &client){
    std::string error_msg = "<h1>File not found</h1>";

    response resp(404, "Not Found");
    resp.add_header("Content-Type: text/html; charset=UTF-8");
    resp.add_header("Content-Length: " + std::to_string(error_msg.length()));

    resp.set_body({error_msg.begin(), error_msg.end()});
    resp.send(client);
}

enum class file_type{
    html, png, other
};

class file_open_error : std::runtime_error {
    std::string path;
public:
    file_open_error(const std::string &msg, std::string path) : std::runtime_error(msg), path(std::move(path)) {}
};

file_type get_file_type(const std::string &path){
    size_t dot = path.find_last_of('.');
    std::string f_type = path.substr(dot+1);

    if ((f_type == "html") or (f_type == "htm")) return file_type::html;
    if (f_type == "png") return file_type::png;

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

void add_body(response &resp, const std::string &path){
    file_type f_type = get_file_type(path);
    size_t body_size;
    switch (f_type) {
        case file_type::html:
            body_size = add_text_body(resp, path);
            resp.add_header("Content-Type: text/html; charset=UTF-8");
            resp.add_header("Content-Length: " + std::to_string(body_size));
            break;
        case file_type::png:
            body_size = add_bin_body(resp, path);
            resp.add_header("Content-Type: image/png");
            resp.add_header("Content-Length: " + std::to_string(body_size));
            break;
        case file_type::other:
            return;
    }
}

//TODO:
//  extend(rewrite) document type checking (for Content-Type header)
void callback(active_socket &client){
    request req = accept_request(client);
    std::cout << req;

    if (req.get_request_method() == request_method::GET){
        std::string path = req.get_path();
        if (path.length() <= 1){
            path = R"(..\www\index.html)";
        } else{
            path = R"(..\www\)" + path.substr(1, path.length());
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
    listen_socket server("127.0.0.1", PORT);
    server.set_callback(&callback);
    server.start_listening(16);
}

void client(){
    active_socket client;
    client.connect("127.0.0.1", PORT);
    char msg[] = "Hello, server!";
    client.send_msg(msg, sizeof(msg));
}

void test(int argc, char **argv){
    if ((argc < 2) or (argv[1][0] == 'c')){
        printf("Client\n");
        client();
    }
    else{
        printf("Server\n");
        server();
    }
}


int main(int argc, char **argv) {
    test(argc, argv);

    return 0;
}
