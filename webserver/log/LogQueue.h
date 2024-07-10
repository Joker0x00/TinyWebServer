//
// Created by wy on 24-7-2.
//

#ifndef TINYWEBSERVER_LOGQUEUE_H
#define TINYWEBSERVER_LOGQUEUE_H

#include <queue>
#include <cassert>
#include <mutex>
#include <condition_variable>
template <typename T>
class LogQueue {
private:
    std::deque<T> log;
    size_t capacity;
    bool deleted;
    std::mutex mtx_;
    std::condition_variable consumer_cv;
    std::condition_variable producer_cv;
public:
    explicit LogQueue(size_t c);
    ~LogQueue();
    void push(const T &data);
    bool pop(T &item);
    size_t size();
    bool empty();
    void onDelete();
    bool full();
    void flush();
};

template<typename T>
void LogQueue<T>::flush() {
    consumer_cv.notify_one();
}

template<typename T>
bool LogQueue<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return log.size() >= capacity;
}

template<typename T>
LogQueue<T>::LogQueue(size_t c): capacity(c), deleted(false) {
    assert(c > 0);
}

template<typename T>
LogQueue<T>::~LogQueue() {
    onDelete();
}

template<typename T>
void LogQueue<T>::onDelete() {
//    deleted = true;
//    int cnt = 0;
//    while(!empty() || cnt == 5) {
//        sleep(1);
//        cnt ++;
//    }
    {
        std::lock_guard<std::mutex> locker(mtx_);
        deleted = true;
        log.clear();
    }
    consumer_cv.notify_one();
    producer_cv.notify_one();
}

template<typename T>
bool LogQueue<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return log.empty();
}

template<typename T>
size_t LogQueue<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return log.size();
}
// 消费者读取日志
template<typename T>
bool LogQueue<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(log.empty()) {
        consumer_cv.wait(locker);
        if (deleted) {
            return false;
        }
    }
    item = log.front();
    log.pop_front();
    producer_cv.notify_one();
    return true;
}

// 生产者插入日志
template<typename T>
void LogQueue<T>::push(const T &data) {
    std::unique_lock<std::mutex> locker(mtx_);
    // 直至log.size() <= capacity  缓冲区未满
    while(log.size() >= capacity) {
        producer_cv.wait(locker);
    }
    log.push_back(data);
    consumer_cv.notify_one();
}



#endif //TINYWEBSERVER_LOGQUEUE_H
