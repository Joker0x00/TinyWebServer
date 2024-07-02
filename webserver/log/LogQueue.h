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
    std::queue<T> log;
    size_t capacity;
    bool deleted;
    std::mutex mtx_;
    std::condition_variable consumer_cv;
    std::condition_variable producer_cv;
public:
    explicit LogQueue(size_t c);
    ~LogQueue();
    void push(T data);
    T pop();
    size_t size();
    bool empty();
    void notifyALL();
    void notifyConsumer();
    void notifyProducer();
    void onDelete();
    void clear();
};

template<typename T>
void LogQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    std::queue<T>t;
    swap(t, log);
}

template<typename T>
void LogQueue<T>::onDelete() {
    deleted = true;
    int cnt = 0;
    while(!empty() || cnt == 5) {
        sleep(1);
        cnt ++;
    }
    notifyALL();
    clear();
}

template<typename T>
void LogQueue<T>::notifyProducer() {
    producer_cv.notify_one();
}

template<typename T>
void LogQueue<T>::notifyConsumer() {
    consumer_cv.notify_one();
}

template<typename T>
void LogQueue<T>::notifyALL() {
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

template<typename T>
T LogQueue<T>::pop() {
    std::unique_lock<std::mutex> locker(mtx_);
    while(log.empty()) {
        consumer_cv.wait(locker);
    }
    T logItem = log.front();
    log.pop();
    producer_cv.notify_one();
    return logItem;
}

// 生产者操作
template<typename T>
void LogQueue<T>::push(T data) {
    std::unique_lock<std::mutex> locker(mtx_);
    // 直至log.size() <= capacity  缓冲区未满
    while(log.size() >= capacity) {
        producer_cv.wait(locker);
    }
    log.push(data);
    consumer_cv.notify_one();
}

template<typename T>
LogQueue<T>::~LogQueue() {
    onDelete();
}

template<typename T>
LogQueue<T>::LogQueue(size_t c):capacity(c), deleted(false) {
    assert(c > 0);
}

#endif //TINYWEBSERVER_LOGQUEUE_H
