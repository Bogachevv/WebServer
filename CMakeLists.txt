cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    link_libraries(ws2_32 wsock32)
    add_executable(WebServer main.cpp socket.h win/socket.cpp win/WSA.h win/WSA.cpp request.h request.cpp response.cpp response.h)
endif(WIN32)

if(UNIX)
    add_executable(WebServer main.cpp socket.h request.cpp request.h response.cpp response.h IO_supervisor.cpp IO_supervisor.h TCP_server.cpp TCP_server.h linux/socket.cpp)
endif(UNIX)