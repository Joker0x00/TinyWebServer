//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_THREADPOOL_H
#define TINYWEBSERVER_THREADPOOL_H
#include <queue>
#include <functional>
#include <mutex>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <unistd.h>
#include "../log/Log.h"
class ThreadPool {
private:
    struct Pool {
        std::mutex mtx_;
        bool isRun = true;
        std::condition_variable cv;
        std::queue<std::function<void()>>taskQueue_;
    };
    std::shared_ptr<Pool> pool_;

//private:
//    static void work(); // 工作函数，从任务队列中取出任务并执行
public:
    explicit ThreadPool(int max_thread_cnt);
    ThreadPool() = default;
    // 定义移动构造函数
    ThreadPool(ThreadPool&&) = default;


    ~ThreadPool();
    bool addTask(std::function<void()> &&f); // 外部调用接口
    void resetTaskQueue();
};


#endif //TINYWEBSERVER_THREADPOOL_H
