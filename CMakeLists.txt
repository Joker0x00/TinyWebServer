cmake_minimum_required(VERSION 3.28)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(TinyWebServer webserver/main.cpp
        webserver/buffer/Buffer.cpp
        webserver/buffer/Buffer.h
        webserver/log/Log.cpp
        webserver/log/Log.h
        webserver/log/LogQueue.h
        webserver/utils/Utils.h
        webserver/log/LogLevel.h
        webserver/pool/ThreadPool.cpp
        webserver/pool/ThreadPool.h
        webserver/http/ParseHttpRequest.cpp
        webserver/http/ParseHttpRequest.h
        webserver/http/HttpMethod.h
        webserver/http/HttpResponse.h
        webserver/http/HttpParams.h
        webserver/http/HttpWork.cpp
        webserver/http/HttpWork.h
        webserver/http/HttpResponse.cpp
        webserver/timer/Timer.cpp
        webserver/timer/Timer.h
        webserver/server/Epoll.cpp
        webserver/server/Epoll.h
        webserver/server/Server.cpp
        webserver/server/Server.h
        webserver/urls/Router.h
        webserver/views/Base.h
        webserver/views/Login.h
        webserver/http/Response.h
        webserver/lib/json.cpp
        webserver/lib/json/json.h
        webserver/lib/json/json-forwards.h
        webserver/http/HttpMethod.cpp
        webserver/urls/Router.cpp
        webserver/views/Index.h
        webserver/Type.h
)
set(CMAKE_CXX_FLAGS -pthread)
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")