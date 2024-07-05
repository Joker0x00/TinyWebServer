//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPWORK_H
#define TINYWEBSERVER_HTTPWORK_H
#include "../buffer/Buffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "HttpResponse.h"
#include "ParseHttpRequest.h"
// 每个工作线程操纵的类接口，负责读写数据，处理Http请求，每个用户持有一个类
class HttpWork {
private:
    Buffer writeBuf_;
    Buffer readBuf_;
    int fd_;
    bool isRun;
    struct sockaddr_in addr_{};
    iovec iv[2]{};
    int io_cnt = 2;

    ParseHttpRequest request_;
    HttpResponse response_;
public:
    static std::string srcDir_;
    static bool et_;
public:
    HttpWork();
    ~HttpWork();
    void init(int fd, const sockaddr_in &addr);
    ssize_t writeFd(int *Errno);
    ssize_t readFd(int *Errno);
    bool processHttp();
    size_t getWriteLen();
    void closeConn();
    bool getIsRun();
    int getFd();
    bool isKeepAlive();
};


#endif //TINYWEBSERVER_HTTPWORK_H
