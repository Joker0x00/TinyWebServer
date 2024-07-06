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

ssize_t HttpWork::readFd(int *Errno) {
    ssize_t len;
    do {
        len = readBuf_.readFd(fd_, Errno);
        Log::DEBUG("read len: %d", len);
        if (len <= 0) {
            break;
        }
    } while(et_);
    return len;
}

ssize_t HttpWork::writeFd(int *Errno) {
    // 有两块数据要写: buf, 文件
    iv[0].iov_base = writeBuf_.getReadPtr();
    iv[0].iov_len = writeBuf_.getContentLen();
    iv[1].iov_base = response_.getFile();
    iv[1].iov_len = response_.getFileLen();
    ssize_t len = 0;
    do {
        len = writev(fd_, iv, io_cnt);
        Log::DEBUG("write len: %d", len);
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
        Log::INFO("%s", "parse request successfully");
//        std::string headers;
//        ParsedUrl *url = request_.getParsedUrl_();
//        for (auto &it : request_.getHeaders()) {
//            headers += it.first + ": " + it.second + "\n";
//        }
//        std:: string params;
//        for (auto &it : url->queryParams) {
//            params += it.first + ": " + it.second + "\n";
//        }
        auto params = request_.getParams();
        auto response = Router::process(params);

//        Log::INFO("\nmethod: %s\nversion: %s\nurl: %s\nheaders:\n%s\nparams:\n%s\nbody: %s", request_.getMethod().c_str(), request_.getVersion().c_str(), url->path.c_str(), headers.c_str(), params.c_str(), request_.getBody().c_str());
        // 生成HTTP Response
        // 无用代码
//        response_.init(request_.getParsedUrl_()->path, srcDir_, request_.keepAlive(), 200);
        // 根据response生成http响应报文，并写入缓冲区，等待服务器发送
        response_.makeResponse(response, request_.keepAlive(), writeBuf_);

    } else {
//        response_.init(request_.getParsedUrl_()->path, srcDir_, request_.keepAlive(), 400);
        std::shared_ptr<Response> r = std::make_shared<Response>(404);
        response_.makeResponse(r, request_.keepAlive(), writeBuf_);
    }
    Log::INFO("making response%s", "");
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
    Log::DEBUG("wait for write response%s", "");
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
