//
// Created by wy on 24-7-4.
//

#include "Timer.h"

#include <utility>

void Timer::up(size_t u) {
    while(u != 1 && heap_[u] < heap_[u/2]) {
        swap_(u, u / 2);
        u /= 2;
    }
}
// 从1开始 1 2 3 4 ... 1是根结点
void Timer::down(size_t u) {
    size_t t = u;
    if (u * 2 <= si_ && heap_[u*2] < heap_[t]) t = u * 2;
    if (u * 2 + 1 <= si_ && heap_[u * 2 + 1] < heap_[t]) t = u * 2 + 1;
    if (t != u) {
        swap_(u, t);
        down(t);
    }
}

void Timer::swap_(size_t t1, size_t t2) {
    assert(t1 >= 1 && t1 <= heap_.size());
    assert(t2 >= 1 && t2 <= heap_.size());
    std::swap(heap_[t1], heap_[t2]);
    ref_[heap_[t1].id_] = t1;
    ref_[heap_[t2].id_] = t2;
}
// 删除顶部结点
void Timer::pop() {
    assert(si_ > 0);
    del(1);
}

TimerNode& Timer::top() {
    return heap_[1];
}

// 删除给定结点i，将其和最后一个结点交换，之后执行up和down操作
void Timer::del(size_t i) {
    assert(i > 0 && i <= si_);
    swap_(i, si_);
    -- si_;
    ref_.erase(heap_.back().id_);
    heap_.pop_back();
    up(i);
    down(i);
}
// 执行id的回调函数
void Timer::execCb(int id) {
    if (si_ == 0 || ref_.count(id) == 0) {
        return ;
    }
    auto idx = ref_[id];
    auto &node = heap_[idx];
    node.cb_();
    del(idx);
}

void Timer::push(int id, int timeout, const TimerCallback &cb) {
    assert(id >= 0);
    if (ref_.count(id)) {
        auto idx = ref_[id];
        auto &node = heap_[idx];
        node.expires_ = Clock::now() + MS(timeout);
        node.cb_ = cb;
        up(idx);
        down(idx);
    } else {
        LOG_INFO("增加计时时间 id: %d timeout: %d", id, timeout);
        heap_.push_back({id, MS(timeout) + Clock::now(), cb});
        ++ si_;
        ref_[id] = si_;
        up(si_);
    }
}

Timer::Timer(size_t cnt) {
//    Log::INFO("%s", "Timer start...");
    heap_.reserve(cnt + 1);
    heap_.emplace_back();
    si_ = 0;
}

void Timer::reset(int id, int timeout, TimerCallback &cb) {
    assert(id >= 0);
    auto idx = ref_[id];
    auto &node = heap_[idx];
    node.expires_ = Clock::now() + MS(timeout);
    node.cb_ = cb;
    down(idx);
    up(idx);
}

void Timer::reset(int id, int timeout) {
    assert(id >= 0);
    auto idx = ref_[id];
    auto &node = heap_[idx];
    node.expires_ = Clock::now() + MS(timeout);
    down(idx);
    up(idx);
}

void Timer::tick() {
    while(si_) {
        auto &node = top();
        if (std::chrono::duration_cast<MS>(node.expires_ - Clock::now()).count() > 0)
            break;
        LOG_INFO("timer %d is expired", node.id_);
        node.cb_();
        pop();
    }
}

bool Timer::empty() {
    return si_ == 0;
}

int Timer::getNextTick() {
    tick();
    size_t res = -1;
    if (si_) {
        res = std::chrono::duration_cast<MS>(top().expires_ - Clock::now()).count();
        if (res < 0) {
            res = 0;
        }

    }
    return res;
}

Timer::~Timer() {
    heap_.clear();
    ref_.clear();
}
