//
// Created by wy on 24-7-4.
//

#include "Server.h"

Server::Server(const char *ip, int port, int trigMod, int timeout, LogTarget target, LogLevel::value logLevel, int max_thread_cnt,
               int max_timer_cnt, int max_fd, int max_epoll_events, int sqlPort, const char *sqlUser,
               const char * sqlPwd, const char * dbName, int connPoolNum):ip_(ip), port_(port),
               trigMod_(trigMod), timeoutMs_(timeout), MAXFD_(max_fd),
               threadPool_(new ThreadPool(max_thread_cnt)), timer_(new Timer(max_timer_cnt)),
               epoll_(new Epoll(max_epoll_events)) {
    isRun_ = false;
    srcDir_ = getcwd(nullptr, 256);
    auto l = Log::getInstance();
//     初始化日志系统
    l->init(target, (srcDir_ + "/log").c_str(), ".log", logLevel);
    HttpWork::srcDir_ = srcDir_ + "/resources";

    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
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
        LOG_FATAL("create socket failed");
        return false;
    }
    setNonBlocking(listenFd_);
    int res;
    int optVal = 1;
    res = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    if(res == -1) {
        LOG_FATAL("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    res = bind(listenFd_, (struct sockaddr*)&address, sizeof address);
    if (res == -1) {
        LOG_FATAL("bind socket failed");
        return false;
    }

    res = listen(listenFd_, 8);
    if (res < 0) {
        LOG_FATAL("%s %d", "listen failed", res);
        return false;
    }

    epoll_->addFd(listenFd_, EPOLLIN|listenEvents_);

    LOG_INFO("listening on %s:%d", ip_, port_);
    return true;
}


int Server::setNonBlocking(int fd) {
    int old = fcntl(fd, F_GETFL);
    int newOp = old | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOp);
    return old;
}

void Server::run() {
    if (!isRun_) {
        LOG_ERROR("Server start failed");
        return;
    }
    int timeout = -1;
    LOG_INFO("Server start running");
    while(isRun_) {
        if (timeoutMs_ > 0) {
            // 清理过期时间
            timeout = timer_->getNextTick();
        }
        // 等待直到下一个定时事件超时，如果timeout为-1代表队列中已经没有定时任务，阻塞等待
        int cnt = epoll_->wait(timeout);
         for (int i = 0; i < cnt; ++ i) {
            // 以此处理每个事件
            int fd = epoll_->getEventFd(i);
            uint32_t events = epoll_->getEvents(i);
            if (fd == listenFd_) {
                // 处理服务器连接请求
                dealListen();
            } else if (events & (EPOLLRDHUP & EPOLLERR & EPOLLHUP)) {
                LOG_WARN("(main): close event: fd(%d)", fd);
                closeConn(users_[fd]); // 关闭连接
            } else if (events & EPOLLIN) {
                dealRead(users_[fd]);
            } else if (events & EPOLLOUT) {
                dealWrite(users_[fd]);
            } else {
                LOG_ERROR("(main): unexpected event");
            }
        }
    }
}

void Server::dealListen() {
    sockaddr_in address{0};
    socklen_t addr_len = sizeof address;
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&address, &addr_len);
        if (fd <= 0) {
            return;
        } else if (HttpWork::userCount >= MAXFD_) {
            sendError(fd, "Server busy");
            LOG_ERROR("server is full");
            return;
        }
        addClient(fd, address);
    } while(listenEvents_ & EPOLLET);
}
void Server::addClient(int fd, sockaddr_in &addr) {
    // 初始化连接
    users_[fd].init(fd, addr);
    HttpWork &client = users_[fd];
//    Log::DEBUG("(main): user %d isRun: %s", fd, std::to_string(client.getIsRun()).c_str());
    setNonBlocking(fd);
    // 假如监听列表
    epoll_->addFd(fd, EPOLLIN|httpConnEvents_);
    // 超时后断开连接
    if (timeoutMs_ > 0) {
        // 添加定时事件u
        timer_->push(fd, timeoutMs_, [this, &client] { closeConn(client); }); // 这里报错了，原因是closeConn的client参数应为指针
    }
    LOG_INFO("(main): user[%d] in, ip: %s, port: %d", fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void Server::dealWrite(HttpWork &client) {
    assert(client.getIsRun());
    extendTime(client.getFd());
    threadPool_->addTask([this, &client] { writeCb(client); });
}

void Server::dealRead(HttpWork &client) {
//    LOG_INFO("(main): dealRead client: %d", client.getFd());
    assert(client.getIsRun());
    extendTime(client.getFd());
    threadPool_->addTask([this, &client] { readCb(client); });
}

void Server::sendError(int fd, const char *msg) {
    assert(fd >= 0);
    auto len = write(fd, msg, sizeof msg);
    if (len <= 0) {
        LOG_WARN("(main): send error to client %d error", fd);
    }
    close(fd);
}


void Server::extendTime(int fd) {
    assert(fd >= 0);
    timer_->reset(fd, timeoutMs_);
}

void Server::readCb(HttpWork &client) {
    assert(client.getIsRun());
    int Errno = 0;

    auto len = client.readFd(&Errno);
    if (len <= 0 && !(Errno == EAGAIN || Errno == 0)) {
        // 出现了其他错误，关闭连接
        LOG_ERROR("(thread):read error: %d, client %d is closing", Errno, client.getFd());
        closeConn(client);
        return;
    }
    if (client.processHttp()) {
        // 成功处理了http读请求，response已生成，等待写出
        epoll_->modFd(client.getFd(), EPOLLOUT | httpConnEvents_);
    } else {
        // http请求未处理，读缓冲为空，重新等待请求
        epoll_->delFd(client.getFd());
        LOG_ERROR("(thread): readBuf is none, client: %d", client.getFd());
        closeConn(client);
    }
}

void Server::writeCb(HttpWork &client) {
    assert(client.getIsRun()); // 连接未关闭
    int Errno = 0;
    auto len = client.writeFd(&Errno);
//    LOG_DEBUG("Error: %d", Errno);
    if (client.getWriteLen() == 0) {
        LOG_INFO("(thread): write successfully from user %d", client.getFd());
        // 传输成功
        if (client.isKeepAlive()) {
            epoll_->modFd(client.getFd(), EPOLLIN | httpConnEvents_);
            client.resetBuffer();
            return;
        }
    } else if (len <= 0 && Errno == EAGAIN) {
        // 写缓冲满了，继续传输
//        LOG_WARN("EAGAIN, continue write, client %d", client.getFd());
        epoll_->modFd(client.getFd(), EPOLLOUT | httpConnEvents_);
        return;
    }
    LOG_INFO("(thread): client %d is closing", client.getFd());
    closeConn(client);
}

void Server::closeConn(HttpWork &client) {
    if (!client.getIsRun())
        return;
    LOG_INFO("(main): client %d is closing", client.getFd());
    epoll_->delFd(client.getFd());
    client.closeConn();
}

Server::~Server() {
    close(listenFd_);
    isRun_ = false;
}