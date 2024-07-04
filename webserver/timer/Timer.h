//
// Created by wy on 24-7-4.
//

#ifndef TINYWEBSERVER_TIMER_H
#define TINYWEBSERVER_TIMER_H
#include <vector>
#include <functional>
#include <cassert>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <atomic>
#include "../log/Log.h"
typedef std::function<void()> TimerCallback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;


// 定时器结点
struct TimerNode {
    int id_; // 定时器id
    TimeStamp expires_; // 定时器过期时间点
    TimerCallback cb_; // 回调函数
    bool operator<(const TimerNode &tn) const {
        return expires_ < tn.expires_;
    }
    bool operator>(const TimerNode &tn) const {
        return expires_ > tn.expires_;
    }
};

// 堆定时器，存储定时事件
class Timer {
private:
    // 定时器堆，采用vector数组方式存储
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;  // 从id到heap中的下标映射，方便直接操作某个node
    std::atomic<size_t> si_;
public:
    explicit Timer(size_t cnt);
    ~Timer();
private:
    void up(size_t u); // 将某个结点向上调整的操作
    void down(size_t u); // 将某个结点向下调整的操作
    void pop(); // 删除堆顶的结点
    TimerNode &top(); // 获得堆顶的结点
    void del(size_t i); // 删除下标为i的结点 并执行回调函数
    void swap_(size_t t1, size_t t2); // 交换两个下标的位置

public:
    int getNextTick(); // 返回最近的定时器事件超时的间隔时间
    void reset(int id, int timeout); // 重新设置某个结点的过期时间
    void reset(int id, int timeout, TimerCallback &cb); // 重设某个结点的过期时间和回调函数
    void push(int id, int timeout, TimerCallback &cb); // 向堆中插入一个新的定时器事件
    void execCb(int id); // 执行id的回调函数
    void tick(); // 处理堆中过期的定时器
    void push(int id, int timeout, const TimerCallback &cb);

    bool empty();
};


#endif //TINYWEBSERVER_TIMER_H
