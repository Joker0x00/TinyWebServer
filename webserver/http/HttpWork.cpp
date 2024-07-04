//
// Created by wy on 24-7-3.
//

#include "HttpWork.h"


void HttpWork::init(bool et, const char* srcDir, int fd, const sockaddr_in &addr) {
    assert(fd > 0);
    et_ = et;
    fd_ = fd;
    addr_ = addr;
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    isRun = true;
    srcDir_ = srcDir;
}

HttpWork::HttpWork() {
    fd_ = -1;
    isRun = false;
    et_ = true;
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

void HttpWork::writeFd(int *Errno) {
    // 有两块数据要写: buf, 文件
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    iv[1].iov_base = response_.getFile();
    iv[1].iov_len = response_.getFileLen();

    do {
        size_t len = writev(fd_, iv, io_cnt);
        if (len <= 0) {
            // 写错误
            *Errno = errno;
            break;
        }
        if (len == getWriteLen()) break; // 写成功
        // 更新写指针
        if (len < iv[0].iov_len) {
            // 此时第一个iovec没有写完
            // 我们将更新iovec的base和buffer中的指针位置
            writeBuf_.addReadIdx(len);
            iv[0].iov_base = writeBuf_.getReadPtr();
            iv[0].iov_len = writeBuf_.getContentLen();
        } else {
            size_t hasWritten = len - iv[0].iov_len;
            iv[1].iov_base = (uint8_t*)iv[1].iov_base + hasWritten;
            iv[1].iov_len -= hasWritten;
            if (iv[0].iov_len) {
                iv[0].iov_base = nullptr;
                iv[0].iov_len = 0;
                writeBuf_.resetBuffer();
            }
        }
    } while(et_);
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
