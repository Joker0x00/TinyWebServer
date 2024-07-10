//
// Created by wy on 24-7-3.
//


#include "ThreadPool.h"

ThreadPool::ThreadPool(int max_thread_cnt): pool_(std::make_shared<Pool>()) {
    assert(max_thread_cnt > 0);
    for (int i = 0; i < max_thread_cnt; ++ i) {
        printf("init thread %d\n", i);
        std::thread([pool = pool_, i]{
            std::unique_lock<std::mutex> locker(pool->mtx_);
            while(true) {
                if (!pool->taskQueue_.empty()) {
                    LOG_INFO("thread pool: thread %d process task", i);
                    // 有任务，开始干活
                    // 这个地方使用move，提高效率
                    auto task = std::move(pool->taskQueue_.front());
                    pool->taskQueue_.pop();
                    locker.unlock();
                    task(); // 处理任务
                    locker.lock();
                } else if (!pool->isRun) {
                    break;
                } else {
                    pool->cv.wait(locker);
                }
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool() {
    if (static_cast<bool>(pool_))
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx_);
            pool_->isRun = false;
        }
        pool_->cv.notify_all();
    }
}

void ThreadPool::resetTaskQueue() {
    std::queue<std::function<void()>> q;
    swap(q, pool_->taskQueue_);
}

bool ThreadPool::addTask(std::function<void()> &&f) {
    {
        std::lock_guard<std::mutex> locker(pool_->mtx_);
        pool_->taskQueue_.emplace(std::forward<std::function<void()>>(f));
    }
    LOG_INFO("thead pool: %s", "add task");
    pool_->cv.notify_one();
    return true;
}
