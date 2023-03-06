#include <iostream>
#include <fstream>
#include <stdexcept>

#include "socket.h"
#include "request.h"
#include "response.h"

#define PORT 127
//TODO:
//  1) implement body transmission
//  2) implement document type checking (for Content-Type header)
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

        std::ifstream file(path, std::ios::binary);
        if (not file.good()){
            std::string error_msg = "<h1>File not found</h1>";

            response resp(404, "Not Found");
            resp.add_header("Content-Type: text/html; charset=UTF-8");
            resp.add_header("Content-Length: " + std::to_string(error_msg.length()));

            resp.set_text_body(error_msg);
            resp.send(client);
            return;
        }

        size_t body_size;
        file.seekg(0, std::ios::end);
        body_size = file.tellg();
        file.seekg(0, std::ios::beg);
        char *body = new char[body_size];

        file.read(body, (std::streamsize)body_size);

        response resp(200, "OK");
        resp.add_header("Content-Type: text/html; charset=UTF-8");
        resp.add_header("Content-Length: " + std::to_string(body_size));

        resp.capture_body(body, body_size);
        resp.send(client);

        std::cout << body;
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
