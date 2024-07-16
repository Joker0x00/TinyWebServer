//
// Created by wy on 24-7-4.
//

#ifndef TINYWEBSERVER_SERVER_H
#define TINYWEBSERVER_SERVER_H
#include "../http/HttpWork.h"
#include "../http/HttpResponse.h"
#include "../http/ParseHttpRequest.h"
#include "../log/Log.h"
#include "../pool/ThreadPool.h"
#include "../timer/Timer.h"
#include "Epoll.h"
#include <unordered_map>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <functional>
#include "../urls/Router.h"


class Server {
private:
    const char *ip_;
    int port_;
    int trigMod_;
    int timeoutMs_;
    int MAXFD_;
    int userCnt;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Timer> timer_;
    std::unique_ptr<Epoll> epoll_;

    std::unordered_map<int, HttpWork> users_; // 负责处理HTTP请求
    uint32_t httpConnEvents_{};
    uint32_t listenEvents_{};
    int listenFd_{};
    bool isRun_;
    std::string log_dir_;
    std::string srcDir_;

public:
    // 提供服务器运行参数
    Server(const char* ip, int port, int trigMod, int timeout, LogTarget target, LogLevel::value logLevel,
           int max_thread_cnt, int max_timer_cnt, int max_fd, int max_epoll_events, int sqlPort, const char * sqlUser,
           const char * sqlPwd, const char * dbName, int connPoolNum);
    ~Server();
    void initTrigMode();
    bool startListen();
    int setNonBlocking(int fd);
    void dealListen();
    void addClient(int fd, sockaddr_in &addr);
    void dealWrite(HttpWork &client);
    void dealRead(HttpWork &client);
    void sendError(int fd, const char *msg);
    void closeConn(HttpWork &client);
    void extendTime(int fd);
    void readCb(HttpWork &client);
    void writeCb(HttpWork &client);
    void run();

    void testRun();
};


#endif //TINYWEBSERVER_SERVER_H
