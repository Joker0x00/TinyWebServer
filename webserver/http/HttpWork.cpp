//
// Created by wy on 24-7-3.
//

#include "HttpWork.h"
bool HttpWork::et_;
std::string HttpWork::srcDir_;
std::atomic<int> HttpWork::userCount;

void HttpWork::init(int fd, const sockaddr_in &addr) {
    assert(fd > 0);
    std::lock_guard<std::mutex> locker(mtx_);
    isRun_ = true;
    fd_ = fd;
    addr_ = addr;
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    request_.init();
    userCount ++;
}

HttpWork::HttpWork() {
    isRun_ = false;
    addr_ = {0};
}

ssize_t HttpWork::readFd(int *Errno) {
    assert(fd_ >= 0);
    ssize_t len = 0;
    do {
        auto t_len = readBuf_.readFd(fd_, Errno);
        // 返回0代表此次读取数据为0
        if (t_len <= 0) {
            break;
        }
        len += t_len;
    } while(et_);
    // len是此次总计读取的数据
    return len;
}

ssize_t HttpWork::writeFd(int *Errno) {
    assert(fd_ >= 0);
    ssize_t len = 0;
    do {
        len = writev(fd_, iv, io_cnt);
        if (len <= 0) {
            // 写错误
            *Errno = errno;
            break;
        }
        // 处理第一个缓冲区
        if (iv[0].iov_len > 0) {
            // 此时第一个iovec没有写完
            // 我们将更新iovec的base和buffer中的指针位置
            auto iv_len1 = writeBuf_.getContentLen(); // 获取待写入数据的长度
            if (iv_len1 <= static_cast<size_t>(len)) { // buf中全部写完
                // iv1已经全部写完，后续不再处理
                iv[0].iov_base = nullptr;
                iv[0].iov_len = 0;
                writeBuf_.resetBuffer();
                len = static_cast<ssize_t>(static_cast<size_t>(len) - iv_len1); // 获取第二个iv结点写入的数据
            } else {
                // iv1写了一部分
                writeBuf_.addReadIdx(len); // 更新
                // 指针
                iv[0].iov_base = writeBuf_.getReadPtr();
                iv[0].iov_len = writeBuf_.getContentLen();
                len = 0;
            }
        }
        // 处理第二个缓冲区
        if (iv[0].iov_len == 0)
        {
            iv[1].iov_base = (uint8_t*)iv[1].iov_base + len;
            iv[1].iov_len -= len;
        }
        if (0 == getWriteLen()) {
            iv[1].iov_base = nullptr;
            iv[1].iov_len = 0;
            break; // 写成功
        }
    } while(et_);
    return len;
}

HttpWork::~HttpWork() {
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    fd_ = -1;
    close(fd_);
}

size_t HttpWork::getWriteLen() {
    return iv[0].iov_len + iv[1].iov_len;
}

void HttpWork::closeConn() {
    std::lock_guard<std::mutex> locker(mtx_);
    if (isRun_) {
        close(fd_);
        fd_ = -1;
        isRun_ = false;
        userCount --;
        LOG_DEBUG("client %d is closed", fd_);
    }
}

bool HttpWork::getIsRun() {
    std::lock_guard<std::mutex> locker(mtx_);
    return isRun_;
}

int HttpWork::getFd() {
    std::lock_guard<std::mutex> locker(mtx_);
    return fd_;
}

bool HttpWork::isKeepAlive() {
    return request_.keepAlive();
}

void HttpWork::resetBuffer() {
    readBuf_.resetBuffer();
    writeBuf_.resetBuffer();
}

bool HttpWork::processHttp() {
    // 读缓冲中没有数据，接下来继续等待读
    if (readBuf_.getContentLen() <= 0) {
        return false;
    }
//    LOG_DEBUG("readBuf: %s", std::string(readBuf_.getConstReadPtr(), readBuf_.getContentLen()).c_str());
    request_.init(); // 清空上一次的数据
    // 请求成功解析
    if (request_.parse(readBuf_)) {
        // 解析成功，正式进入业务逻辑处理流程
        response_.init(srcDir_, request_.path(), request_.keepAlive(), 200);
    } else {
        response_.init(srcDir_, request_.path(), false, 400);
    }
    LOG_INFO("%s %s", request_.method().c_str(), request_.path().c_str());
    response_.makeResponse(writeBuf_);
    // 输出报文11
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    io_cnt = 1;
    if (response_.file() && response_.fileLen() > 0) {
        iv[1].iov_base = response_.file();
        iv[1].iov_len = response_.fileLen();
        io_cnt = 2;
    }
//    LOG_DEBUG("wait for write data: %d", getWriteLen());
    // 返回true表示等待写
    return true;
}