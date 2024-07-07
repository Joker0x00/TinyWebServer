//
// Created by wy on 24-7-4.
//

#include "Server.h"

Server::Server(const char *ip, int port, int trigMod, int timeout, LogTarget target, LogLevel::value logLevel, int max_thread_cnt,
               int max_timer_cnt, int max_fd, int max_epoll_events, const std::string &srcDir):ip_(ip), port_(port),
               trigMod_(trigMod), timeoutMs_(timeout), MAXFD_(max_fd), userCnt(0),
               threadPool_(new ThreadPool(max_thread_cnt)), timer_(new Timer(max_timer_cnt)),
               epoll_(new Epoll(max_epoll_events)) {
    isRun_ = false;
//    if (srcDir == "")
//        srcDir = std::string(getcwd(nullptr, 0));
    auto l = Log::getInstance();
//     初始化日志系统
    l->init(target, (srcDir + "/log").c_str(), ".txt", logLevel);
    HttpWork::srcDir_ = srcDir + "/html";

//     初始化监听事件
    initTrigMode();
//     启动listenFd
    if (startListen()) {
        isRun_ = true;
    }
}

void Server::initTrigMode() {
    listenEvents_ = EPOLLRDHUP;    // 检测socket关闭
    httpConnEvents_ = EPOLLONESHOT | EPOLLRDHUP;     // EPOLLONESHOT由一个线程处理
    switch (trigMod_) {
        case 0:
            break;
        case 1:
            httpConnEvents_ |= EPOLLET;
            break;
        case 2:
            listenEvents_ |= EPOLLET;
            break;
        case 3:
            listenEvents_ |= EPOLLET;
            httpConnEvents_ |= EPOLLET;
            break;
        default:
            listenEvents_ |= EPOLLET;
            httpConnEvents_ |= EPOLLET;
    }
    HttpWork::et_ = (httpConnEvents_ & EPOLLET);
}

bool Server::startListen() {
    struct sockaddr_in address = {0};
    address.sin_port = htons(port_);
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip_, &address.sin_addr);

    listenFd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        Log::FATAL("%s", "create socket failed");
        return false;
    }
    setNonBlocking(listenFd_);
    int res;
    int optVal = 1;
    res = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    if(res == -1) {
        Log::FATAL("%s", "set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    res = bind(listenFd_, (struct sockaddr*)&address, sizeof address);
    if (res == -1) {
        Log::FATAL("%s", "bind socket failed");
        return false;
    }

    res = listen(listenFd_, 8);
    if (res < 0) {
        Log::FATAL("%s %d", "listen failed", res);
        return false;
    }

    epoll_->addFd(listenFd_, EPOLLIN|listenEvents_);

    Log::INFO("listening on %s:%d", ip_, port_);
    return true;
}


int Server::setNonBlocking(int fd) {
    int old = fcntl(fd, F_GETFL);
    int newOp = old | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOp);
    return old;
}

void Server::testRun(){
    listenFd_ = socket(PF_INET, SOCK_STREAM, 0);
    struct linger tmp = {1, 0};
    setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof tmp);
    int ret = 0;

    struct sockaddr_in address;
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    inet_pton(AF_INET, ip_, &address.sin_addr);

    ret = bind(listenFd_, (struct sockaddr*)&address, sizeof address);
    assert(ret >= 0);
    ret = listen(listenFd_, 5);
    assert(ret >= 0);

    int timeout = -1;
    auto efd = epoll_create(5);
    epoll_event events[5];
    epoll_event ev;
    ev.events = EPOLLIN|EPOLLET|EPOLLRDHUP;
    ev.data.fd = listenFd_;
    epoll_ctl(efd, EPOLL_CTL_ADD, listenFd_, &ev);
    int cnt = epoll_wait(efd, events, 5, -1);
    if ((cnt < 0) && (errno != EINTR)) {
        printf("epoll failed\n");
    }
    fflush(stdout);
}

void Server::run() {
    if (!isRun_) {
        Log::ERROR("%s", "Server start failed");
        return;
    }
    int timeout = -1;
    Log::INFO("%s", "Server start running");
    while(isRun_) {
        if (timeoutMs_ > 0) {
            // 清理过期时间
            timeout = timer_->getNextTick();
            Log::INFO("Server: timeout: %d", timeout);
        }
        // 等待直到下一个定时事件超时，如果timeout为-1代表队列中已经没有定时任务，阻塞等待
        int cnt = epoll_->wait(timeout);
         for (int i = 0; i < cnt; ++ i) {
            // 以此处理每个事件
            int fd = epoll_->getEventFd(i);
            uint32_t events = epoll_->getEvents(i);
            if (fd == listenFd_) {
                Log::INFO("listen event: fd(%d)", fd);
                // 处理服务器连接请求
                dealListen();
            } else if (events & (EPOLLRDHUP & EPOLLERR & EPOLLHUP)) {
                assert(users_.count(fd));
                Log::INFO("close event: fd(%d)", fd);
                closeConn(users_[fd]); // 关闭连接
            } else if (events & EPOLLIN) {
                assert(users_.count(fd));
                Log::INFO("read event: fd(%d)", fd);
                dealRead(users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd));
                Log::INFO("write event: fd(%d)", fd);
                dealWrite(users_[fd]);
            } else {
                Log::ERROR("%s", "unexpected event");
            }
        }
    }
}

void Server::closeConn(HttpWork &client) {
    if (!client.getIsRun())
        return;
    userCnt --;
    Log::INFO("user %d is closed", client.getFd());
    client.closeConn();
}

void Server::dealListen() {
    sockaddr_in address{0};
    socklen_t addr_len = sizeof address;
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&address, &addr_len);
        if (fd <= 0) {
            return;
        } else if (userCnt >= MAXFD_) {
            sendError(fd, "Server busy");
            Log::WARN("%s", "server is full");
            return;
        }
        addClient(fd, address);
    } while(listenEvents_ & EPOLLET);
}
void Server::addClient(int fd, sockaddr_in &addr) {
    userCnt ++;
    // 初始化连接
    users_[fd].init(fd, addr);
    HttpWork &client = users_[fd];
    setNonBlocking(fd);
    // 假如监听列表
    epoll_->addFd(fd, EPOLLIN|httpConnEvents_);
    // 超时后断开连接
    if (timeoutMs_ > 0) {
        // 添加定时事件u
        timer_->push(fd, timeoutMs_, [this, &client] { closeConn(client); }); // 这里报错了，原因是closeConn的client参数应为指针
    }
    Log::INFO("user[%d] in, ip: %s, port: %d", fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void Server::dealWrite(HttpWork &client) {
    assert(client.getIsRun());
    extendTime(client.getFd());
    threadPool_->addTask([this, &client] { writeCb(client); });
}

void Server::dealRead(HttpWork &client) {
    assert(client.getIsRun());
    extendTime(client.getFd());
    threadPool_->addTask([this, &client] { readCb(client); });
}

void Server::sendError(int fd, const char *msg) {
    assert(fd >= 0);
    auto len = write(fd, msg, sizeof msg);
    if (len <= 0) {
        Log::WARN("send error to client %d error", fd);
    }
    close(fd);
}


void Server::extendTime(int fd) {
    assert(users_.count(fd));
    timer_->reset(fd, timeoutMs_);
}

void Server::readCb(HttpWork &client) {
    assert(client.getIsRun());
    int Errno = 0;
    auto len = client.readFd(&Errno);
    if (len <= 0 && Errno != EAGAIN) {
        // 出现了其他错误
        closeConn(client);
        return;
    }
    Log::INFO("thread pool: read request from user %d", client.getFd());
    if (client.processHttp()) {
        // 成功处理了http读请求，response已生成，等待写出
        epoll_->modFd(client.getFd(), EPOLLOUT | httpConnEvents_);
        Log::INFO("thread pool: process request from user %d", client.getFd());
    } else {
        // http请求未处理，读缓冲为空，重新等待请求
        epoll_->modFd(client.getFd(), EPOLLIN | httpConnEvents_);
    }
}

void Server::writeCb(HttpWork &client) {
    assert(client.getIsRun()); // 连接未关闭
    int Errno = 0;
    auto len = client.writeFd(&Errno);
    if (client.getWriteLen() == 0) {
        Log::INFO("thread pool: write successfully from user %d", client.getFd());
        // 传输成功
        if (client.isKeepAlive()) {
            epoll_->modFd(client.getFd(), EPOLLIN | httpConnEvents_);
            return;
        }
    } else if (len <= 0 && Errno == EAGAIN) {
        // 写缓冲满了，继续传输
        epoll_->modFd(client.getFd(), EPOLLOUT | httpConnEvents_);
        return;
    }
    closeConn(client);
}


