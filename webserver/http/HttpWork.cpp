//
// Created by wy on 24-7-3.
//

#include "HttpWork.h"
bool HttpWork::et_;
std::string HttpWork::srcDir_;

void HttpWork::init(int fd, const sockaddr_in &addr) {
//    std::lock_guard<std::mutex> locker(mtx_);
    assert(fd > 0);
    fd_ = fd;
    addr_ = addr;
    writeBuf_.resetBuffer();
    readBuf_.resetBuffer();
    request_.init();
//    isRun_ = true;
    LOG_INFO("client %d init", fd);
}

HttpWork::HttpWork() {
//    isRun_ = false;
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
        LOG_DEBUG("(thread): client %d read len: %d", fd_, len);
    } while(et_);
    // len是此次总计读取的数据
    return len;
}

ssize_t HttpWork::writeFd(int *Errno) {
    assert(fd_ >= 0);
    // 有两块数据要写: buf, 文件
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
//    iv[1].iov_base = response_.getFile();
//    iv[1].iov_len = response_.getFileLen();
    ssize_t len = 0;
    do {
        len = writev(fd_, iv, io_cnt);
        LOG_DEBUG("write len: %d", len);
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
            auto iv_len1 = writeBuf_.getContentLen(); // 获取待写入数据的长度
            if (iv_len1 >= len) { // buf中全部写完
                // iv1已经全部写完，后续不再处理s
                iv[0].iov_base = nullptr;
                iv[0].iov_len = 0;
                writeBuf_.resetBuffer();
                len2 = iv_len1 - static_cast<size_t>(len); // 获取第二个iv结点写入的数据
            } else {
                // iv1写了一部分
                writeBuf_.addReadIdx(len); // 更新指针
                iv[0].iov_base = writeBuf_.getReadPtr();
                iv[0].iov_len = writeBuf_.getContentLen();
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
        // 解析成功，正式进入业务逻辑处理流程
        LOG_INFO("%s", "parse request successfully");
        auto params = request_.getParams();
        auto response = Router::process(params);
        HttpResponse::makeResponse(params.method_, 200, response, request_.keepAlive(), writeBuf_);
    } else {
        auto params = request_.getParams();
        auto response = Response::getResponse(400, "HTTP请求解析失败");
        HttpResponse::makeResponse(params.method_, 400, response, request_.keepAlive(), writeBuf_);
    }
    LOG_INFO("making response%s", "");
//    response_.makeResponse(writeBuf_);
    // 输出报文11
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    io_cnt = 1;

//    if (response_.getFile() && response_.getFileLen() > 0) {
//        iv[1].iov_base = response_.getFile();
//        iv[1].iov_len = response_.getFileLen();
//        io_cnt = 2;
//    }
    LOG_DEBUG("wait for write response%s", "");
    // 返回true表示等待写
    return true;
}

void HttpWork::closeConn() {
//    std::lock_guard<std::mutex> locker(mtx_);
//    if (isRun_) {
//        close(fd_);
//        isRun_ = false;
//        Log::DEBUG("client %d is closed", fd_);
//    }
    // 重复关闭会出错，但不做处理，一个socket会多次处理
    close(fd_);
//    isRun_ = false;
    LOG_DEBUG("client %d is closed", fd_);
}

//bool HttpWork::getIsRun() {
////    std::lock_guard<std::mutex> locker(mtx_);
//    return isRun_;
//}

int HttpWork::getFd() {
//    std::lock_guard<std::mutex> locker(mtx_);
    return fd_;
}

bool HttpWork::isKeepAlive() {
//    std::lock_guard<std::mutex> locker(mtx_);
    return request_.keepAlive();
}
