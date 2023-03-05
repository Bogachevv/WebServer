#include <iostream>
#include <stdexcept>

#include "socket.h"
#include "request.h"
#include "response.h"

#define PORT 127

void callback(active_socket &client){
    int counter = 0;
    while (counter < 1){
        std::string str = client.read_line('\n');
        std::cout << str << std::endl;
        if (str[0] == '\r') ++counter;
    }

    std::string resp = "HTTP/1.1 200 OK\n";
    resp += "Content-Type: text/html\n";
    resp += "Content-Length: 20\n";
    resp += "\n";
    resp += "<h1>Hello world</h1>";
    resp += "\n\n";

    client << resp;
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
