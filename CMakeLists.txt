cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(WebServer main.cpp socket.cpp socket.h)
