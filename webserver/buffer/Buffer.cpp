//
// Created by wy on 24-7-1.
//

#include "Buffer.h"

Buffer::Buffer(int init_size, int stack_len):buffer_(init_size), readIdx_(0), writeIdx_(0), STACK_LEN(stack_len){}

size_t Buffer:: getContentLen() {
    return writeIdx_ - readIdx_;
}

size_t Buffer::getBufferLen() {
    return buffer_.size();
}


size_t Buffer::getLeftLen() {
    return getBufferLen() - (getWritePtr() - getBeginPtr());
}

size_t Buffer::getRealLeftLen() {
    return getBufferLen() - getContentLen();
}

char *Buffer::getReadPtr() {
    return &buffer_[readIdx_];
}

char *Buffer::getWritePtr() {
    return &buffer_[writeIdx_];
}

ssize_t Buffer::readFd(int fd, int* Errno) {
    char stack_buf[STACK_LEN];
    iovec iv[2];
    size_t leftLen = getLeftLen();
    iv[0].iov_base = getWritePtr();
    iv[0].iov_len = leftLen;
    iv[1].iov_base = stack_buf;
    iv[1].iov_len = STACK_LEN;

    ssize_t len = readv(fd, iv, 2);
    if (len < 0) {
        // 记录报错信息
        *Errno = errno;
        return len;
    }
    // 刚好将buffer填满
    else if (static_cast<size_t>(len) <= leftLen) {
        addWriteIdx(len);
    }
    // 读入的数据超过buffer
    else {
        writeIdx_ = getBufferLen();
        // 将栈区的数据复制到buffer中
        append(stack_buf, len - static_cast<ssize_t>(leftLen));
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int* Errno) {
    ssize_t len = write(fd, getReadPtr(), getContentLen());
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    addReadIdx(len);
    return len;
}

char* Buffer::getBeginPtr() {
    return &buffer_[0];
}

bool Buffer::confirmSpace(size_t len) {
    // 剩余空间能够满足写入len
    if (getLeftLen() >= len) {
        return false;
    }
    // 不够Len，但是能够借用预留空间满足要求
    else if (getRealLeftLen() >= len) {
        auto contentLen = getContentLen();
        std::copy(getBeginPtr() + readIdx_, getBeginPtr() + writeIdx_, getBeginPtr());
        readIdx_ = 0;
        writeIdx_ = contentLen;
        assert(contentLen == getContentLen());
    }
    // 即使挪动也不够空间，需要对vector扩容
    else {
        buffer_.resize(writeIdx_ + len + 1);
    }
    return true;
}

void Buffer::append(const char* str, size_t len) {
    assert(str);
    confirmSpace(len);
    std::copy(str, str+len, getWritePtr());
    addWriteIdx(len);
}

void Buffer::addWriteIdx(size_t len) {
    writeIdx_ += len;
}

void Buffer::addReadIdx(size_t len) {
    readIdx_ += len;
}

std::string Buffer::getStringAndReset() {
    std::string str(getReadPtr(), getWritePtr());
    resetBuffer();
    return str;
}

void Buffer::resetBuffer() {
    writeIdx_ = 0;
    readIdx_ = 0;
    memset(&buffer_[0], 0, buffer_.size());
}

void Buffer::addReadIdxUntil(const char *ed) {
    assert(getReadPtr() <= ed && ed <= getEndPtr());
    addReadIdx(ed - getReadPtr());
}

char *Buffer::getEndPtr() {
    return &buffer_[buffer_.size() - 1];
}

void Buffer::append(const std::string &str) {
    append(str.c_str(), str.length());
}




