//
// Created by wy on 24-7-3.
//


#include "ThreadPool.h"

ThreadPool::ThreadPool(int max_thread_cnt) {
    MAX_THREAD_CNT = max_thread_cnt;
    isRun = true;
    assert(max_thread_cnt > 0);
    thread_vec.reserve(max_thread_cnt);
    for (int i = 0; i < max_thread_cnt; ++ i) {
        printf("init thread %d\n", i);
        thread_vec[i] = std::thread([this, i]{
            std::unique_lock<std::mutex> locker(mtx_);
            while(true) {
                if (!taskQueue_.empty()) {
                    Log::INFO("thread pool: thread %d process task", i);
                    // 有任务，开始干活
                    auto task = taskQueue_.front();
                    taskQueue_.pop();
                    locker.unlock();
                    task(); // 处理任务
                    locker.lock();
                } else if (!isRun) {
                    break;
                } else {
                    cv.wait(locker);
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        isRun = false;
        cv.notify_all(); // 唤醒所有线程
    }
    for (int i = 0; i < MAX_THREAD_CNT; ++ i) {
        thread_vec[i].join();
    }
}

void ThreadPool::resetTaskQueue() {
    std::queue<std::function<void()>> q;
    swap(q, taskQueue_);
}

bool ThreadPool::addTask(std::function<void()> &&f) {
    std::lock_guard<std::mutex> locker(mtx_);
    Log::INFO("thead pool: %s", "add task");
    taskQueue_.emplace(std::forward<std::function<void()>>(f));
    cv.notify_one();
    return false;
}
