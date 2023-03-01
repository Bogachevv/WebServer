#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <sys/socket.h>
#include "sys/types.h"
#include <netinet/in.h>
#include <unistd.h>
#include <cinttypes>

#include "socket.h"

#define PORT 123

void callback(active_socket &client){
    while (1) {
        std::string str;
        client >> str;
        if (str.length() == 0) break;
        std::cout << str << std::endl;
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
