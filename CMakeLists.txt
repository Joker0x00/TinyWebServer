cmake_minimum_required(VERSION 3.28)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(TinyWebServer main.cpp
        webserver/buffer/Buffer.cpp
        webserver/buffer/Buffer.h)
