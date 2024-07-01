//
// Created by wy on 24-7-1.
//

#ifndef TINYWEBSERVER_BUFFER_H
#define TINYWEBSERVER_BUFFER_H
#include <vector>
#include <atomic> // atomic
#include <sys/uio.h> // iovec readv
#include <cstring> // errno
#include <iostream>
#include <cassert> // assert
#include <unistd.h> // write

class Buffer {
private:
    std::vector<char> buffer_;
    std::atomic<size_t> readIdx_;
    std::atomic<size_t> writeIdx_;
    int STACK_LEN;

public:
    explicit Buffer(int init_size=1024, int stack_len=4096);
    ~Buffer()=default;

    size_t getContentLen();

    size_t getBufferLen();

    size_t getLeftLen();

    size_t getRealLeftLen();

    char* getReadPtr();

    char* getWritePtr();

    char* getEndPtr();

    ssize_t readFd(int fd, int* Errno);

    ssize_t writeFd(int fd, int* Errno);
    // 将
    void append(const char* str, size_t len);

    void addWriteIdx(size_t len);

    void addReadIdx(size_t len);

    void addReadIdxUntil(const char* ed);
    // memset缓冲区
    void resetBuffer();

    std::string getStringAndReset();
private:
    char* getBeginPtr();
    bool confirmSpace(size_t len);
};


#endif //TINYWEBSERVER_BUFFER_H
