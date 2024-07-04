//
// Created by wy on 24-7-4.
//

#include "Epoll.h"

Epoll::Epoll(int maxEvents): events(maxEvents), epollFd(epoll_create(maxEvents)) {
    assert(epollFd >= 0 && !events.empty());
}

Epoll::~Epoll() {
    epollFd = -1;
    events.clear();
}

bool Epoll::delFd(int fd) {
    if (fd < 0) return false;
    epoll_ctl(epollFd, fd, EPOLL_CTL_DEL, nullptr);
    return true;
}

bool Epoll::addFd(int fd, uint32_t ev) {
    if (fd < 0) return false;
    struct epoll_event event = {0};
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(epollFd, fd, EPOLL_CTL_ADD, &event);
    return true;
}

bool Epoll::modFd(int fd, uint32_t ev) {
    if (fd < 0) return false;
    struct epoll_event event = {0};
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(epollFd, fd, EPOLL_CTL_MOD, &event);
    return false;
}

int Epoll::wait() {
    return epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), -1);
}

uint32_t Epoll::getEvents(size_t i) {
    assert(i >= 0 && i < events.size());
    return events[i].events;
}

int Epoll::getEventFd(size_t i) {
    assert(i >= 0 && i < events.size());
    return events[i].data.fd;
}



