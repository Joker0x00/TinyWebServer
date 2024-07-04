//
// Created by wy on 24-7-4.
//

#ifndef TINYWEBSERVER_EPOLL_H
#define TINYWEBSERVER_EPOLL_H

#include <vector>
#include <sys/epoll.h>
#include <cassert>
class Epoll {
private:
    std::vector<struct epoll_event> events;
    int epollFd;
public:
    explicit Epoll(int maxEvents);
    ~Epoll();

    bool addFd(int fd, uint32_t ev);
    bool modFd(int fd, uint32_t ev);
    bool delFd(int fd);

    int wait();
    int getEventFd(size_t i);
    uint32_t getEvents(size_t i);
};


#endif //TINYWEBSERVER_EPOLL_H
