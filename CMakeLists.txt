cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    link_libraries(ws2_32 wsock32)
    add_executable(WebServer main.cpp socket.h win/socket.cpp win/WSA.h win/WSA.cpp request.h request.cpp response.cpp response.h)
endif(WIN32)

if(UNIX)
    add_executable(WebServer main.cpp socket.h linux/socket.cpp request.cpp request.h response.cpp response.h linux/CGI.cpp CGI.h)
endif(UNIX)