//
// Created by wy on 24-7-3.
//

#include "HttpWork.h"
bool HttpWork::et_;
std::string HttpWork::srcDir_;

void HttpWork::init(int fd, const sockaddr_in &addr) {
    assert(fd > 0);
    fd_ = fd;
    addr_ = addr;
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    request_.init();
    isRun = true;
}

HttpWork::HttpWork() {
    fd_ = -1;
    isRun = false;
    memset(&addr_, 0, sizeof addr_);
}

size_t HttpWork::readFd(int *Errno) {
    size_t len;
    do {
        len = readBuf_.readFd(fd_, Errno);
        if (len <= 0) {
            break;
        }
    } while(et_);
    return len;
}

size_t HttpWork::writeFd(int *Errno) {
    // 有两块数据要写: buf, 文件
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    iv[1].iov_base = response_.getFile();
    iv[1].iov_len = response_.getFileLen();
    size_t len = 0;
    do {
        len = writev(fd_, iv, io_cnt);
        if (len <= 0) {
            // 写错误
            *Errno = errno;
            break;
        }
        // 更新写指针
//        if (iv[0].iov_len > 0 && len < iv[0].iov_len) {
//            // 此时第一个iovec没有写完
//            // 我们将更新iovec的base和buffer中的指针位置
//            writeBuf_.addReadIdx(len);
//            iv[0].iov_base = writeBuf_.getReadPtr();
//            iv[0].iov_len = writeBuf_.getContentLen();
//        } else {
//            size_t hasWritten = len - iv[0].iov_len;
//            iv[1].iov_base = (uint8_t*)iv[1].iov_base + hasWritten;
//            iv[1].iov_len -= hasWritten;
//            if (iv[0].iov_len == 0) {
//                iv[0].iov_base = nullptr;
//                iv[0].iov_len = 0;
//                writeBuf_.resetBuffer();
//            }
//        }
        size_t len2 = 0;
        // 处理第一个缓冲区
        if (iv[0].iov_len > 0) {
            // 此时第一个iovec没有写完
            // 我们将更新iovec的base和buffer中的指针位置
            auto iv_len1 = writeBuf_.getContentLen();
            iv_len1 -= len; // 还剩多少没写，可以为负
            if (iv_len1 <= 0) {
                // iv1已经全部写完，后续不再处理s
                iv[0].iov_base = nullptr;
                iv[0].iov_len = 0;
                writeBuf_.resetBuffer();
                len2 = -iv_len1;
            } else {
                // iv1写了一部分
                writeBuf_.addReadIdx(len);
                iv[0].iov_base = writeBuf_.getReadPtr();
                iv[0].iov_len = iv_len1;
            }
        }
        // 处理第二个缓冲区
        if (iv[0].iov_len == 0)
        {
            iv[1].iov_base = (uint8_t*)iv[1].iov_base + len2;
            iv[1].iov_len -= len2;
        }
        if (0 == getWriteLen()) {
            break; // 写成功
        }
    } while(et_);
    return len;
}

HttpWork::~HttpWork() {
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    fd_ = -1;
}

size_t HttpWork::getWriteLen() {
    return iv[0].iov_len + iv[1].iov_len;
}

bool HttpWork::processHttp() {
    // 读缓冲中没有数据，接下来继续等待读
    if (readBuf_.getContentLen() <= 0) {
        return false;
    }
    request_.init(); // 清空上一次的数据
    // 请求成功解析
    if (request_.parse(readBuf_)) {
        response_.init(request_.getParsedUrl_()->path, srcDir_, request_.keepAlive(), 200);
    } else {
        response_.init(request_.getParsedUrl_()->path, srcDir_, request_.keepAlive(), 400);
    }
    response_.makeResponse(writeBuf_);
    // 输出报文
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    io_cnt = 1;

    if (response_.getFile() && response_.getFileLen() > 0) {
        iv[1].iov_base = response_.getFile();
        iv[1].iov_len = response_.getFileLen();
        io_cnt = 2;
    }
    // 返回true表示等待写
    return true;
}

void HttpWork::closeConn() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
    isRun = false;
}

bool HttpWork::getIsRun() {
    return isRun;
}

int HttpWork::getFd() {
    return fd_;
}

bool HttpWork::isKeepAlive() {
    return request_.keepAlive();
}
