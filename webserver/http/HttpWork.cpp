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
//    assert(fd_ >= 0);
//    // 有两块数据要写: buf, 文件
//    iv[0].iov_base = writeBuf_.getReadPtr();
//    iv[0].iov_len = writeBuf_.getContentLen();
//    iv[1].iov_base = response_.file();
//    iv[1].iov_len = response_.fileLen();
//    ssize_t len = 0;
//    do {
//        len = writev(fd_, iv, io_cnt);
//        LOG_DEBUG("write len: %d", len);
//        if (len <= 0) {
//            // 写错误
//            *Errno = errno;
//            break;
//        }
//        // 处理第一个缓冲区
//        if (iv[0].iov_len > 0) {
//            // 此时第一个iovec没有写完
//            // 我们将更新iovec的base和buffer中的指针位置
//            auto iv_len1 = writeBuf_.getContentLen(); // 获取待写入数据的长度
//            LOG_DEBUG("iv_len1: %d", iv_len1);
//            if (iv_len1 <= static_cast<size_t>(len)) { // buf中全部写完
//                // iv1已经全部写完，后续不再处理s
//                iv[0].iov_base = nullptr;
//                iv[0].iov_len = 0;
//                writeBuf_.resetBuffer();
//                len = static_cast<ssize_t>(static_cast<size_t>(len) - iv_len1); // 获取第二个iv结点写入的数据
//            } else {
//                // iv1写了一部分
//                writeBuf_.addReadIdx(len); // 更新
//                // 指针
//                iv[0].iov_base = writeBuf_.getReadPtr();
//                iv[0].iov_len = writeBuf_.getContentLen();
//                len = 0;
//            }
//            LOG_DEBUG("iov0: %d", iv[0].iov_len);
//        }
//        // 处理第二个缓冲区
//        if (iv[0].iov_len == 0)
//        {
//            iv[1].iov_base = (uint8_t*)iv[1].iov_base + len;
//            iv[1].iov_len -= len;
//        }
//        LOG_DEBUG("iov1: %d", iv[1].iov_len);
//        if (0 == getWriteLen()) {
//            LOG_DEBUG("finish write");
//            iv[1].iov_base = nullptr;
//            iv[1].iov_len = 0;
//            break; // 写成功
//        }
//    } while(et_);
//    return len;
    ssize_t len = -1;
    do {
        len = writev(fd_, iv, io_cnt);
        if(len <= 0) {
            *Errno = errno;
            break;
        }
        if(iv[0].iov_len + iv[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iv[0].iov_len) {
            iv[1].iov_base = (uint8_t*) iv[1].iov_base + (len - iv[0].iov_len);
            iv[1].iov_len -= (len - iv[0].iov_len);
            if(iv[0].iov_len) {
                writeBuf_.resetBuffer();
                iv[0].iov_len = 0;
            }
        }
        else {
            iv[0].iov_base = (uint8_t*)iv[0].iov_base + len;
            iv[0].iov_len -= len;
            writeBuf_.addWriteIdx(len);
        }
    } while(et_ || getWriteLen() > 10240);
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
    LOG_DEBUG("readBuf: %s", std::string(readBuf_.getConstReadPtr(), readBuf_.getContentLen()).c_str());
    request_.init(); // 清空上一次的数据
    // 请求成功解析
    if (request_.parse(readBuf_)) {
        // 解析成功，正式进入业务逻辑处理流程
        response_.init(srcDir_, request_.getParams().url_, request_.keepAlive(), 200);
    } else {
        response_.init(srcDir_, request_.getParams().url_, false, 400);
    }
//    readBuf_.resetBuffer();
    LOG_INFO("%s %s", HttpMethod::toStr(request_.getParams().method_).c_str(), request_.getParams().url_.c_str());
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
    LOG_DEBUG("wait for write data: %d", getWriteLen());
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
//    LOG_DEBUG("client %d is closed", fd_);
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

void HttpWork::resetBuffer() {
    readBuf_.resetBuffer();
    writeBuf_.resetBuffer();
}
