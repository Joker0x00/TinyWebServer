//
// Created by wy on 24-7-2.
//

#ifndef TINYWEBSERVER_LOG_H
#define TINYWEBSERVER_LOG_H

#include <string>
#include <thread>
#include <mutex>
#include "../buffer/Buffer.h"
#include "LogQueue.h"
#include "../utils/Utils.h"
#include "LogLevel.h"

// 日志输出位置
enum LogTarget {
    LOG_TARGET_NONE = 0x00,
    LOG_TARGET_CONSOLE = 0x01,
    LOG_TARGET_FILE = 0x10
};


class Log {
private:
    const char* saveDir_; // 日志存储路径
    char* filename_; // 初始化提供的文件名
    const char* suffix_; // 日志文件名后缀
    std::unique_ptr<LogQueue<std::string>> log_Queue_; // 日志队列
    std::unique_ptr<std::thread> workThread_; // 处理写日志的线程
    FILE *fp_; // 日志文件描述符
    LogLevel::value logLevel_;
    LogTarget logTarget_;
    std::mutex mtx_;
    Buffer buf_;
    bool isRun;

    static const size_t MAX_FILENAME_LEN = 50; // 最大文件名限制
public:

    void init(const char* save_dir, const char* suffix, LogLevel::value logLevel,
              size_t maxQueueSize =  1024); // 初始化日志系统
    static void asyncWriteLogThread(); // 工作线程将日志异步写入文件的函数
    bool initLogFile(); // 初始化日志文件
    // 外部调用接口，输出不同类型的日志信息
    template <class ...Args>
    static void addLog(LogLevel::value type, const char *format, Args... args);
    template <class ...Args>
    static void DEBUG(const char *format, Args... args);
    template <class ...Args>
    static void INFO(const char *format, Args... args);
    template <class ...Args>
    static void WARN(const char *format, Args... args);
    template <class ...Args>
    static void ERROR(const char *format, Args... args);
    template <class ...Args>
    static void FATAL(const char *format, Args... args);
    // 外部获取实例的接口
    static Log* getInstance();

private:
    Log();
    ~Log();
    void setEntryTime(); // 设置日志条目时间头
    void setEntryType(LogLevel::value t); // 设置日志条目类型
    void setEntryMsg(const std::string &msg);
    void appendEntry(const std::string& entry);
    void writeFile(const std::string &data);
};

template <class ...Args>
void Log::addLog(LogLevel::value type, const char *format, Args... args) {
    Log *l = Log::getInstance();
    if (type < l->logLevel_) return; // 过滤日志
    std::string message;
    util::String::formatPrintStr(message, format, args...);

    // 向buf_中添加数据，如果多线程访问需要确保只有一个线程访问buf_
    std::unique_lock<std::mutex> locker(l->mtx_);
    l->setEntryType(type);
    l->setEntryTime();
    l->setEntryMsg(message);
    auto entry = l->buf_.getStringAndReset();
    locker.unlock();

    l->appendEntry(entry);
}

template<class... Args>
void Log::DEBUG(const char *format, Args... args) {
    addLog(LogLevel::value::DEBUG, format, args...);
}

template<class... Args>
void Log::INFO(const char *format, Args... args) {
    addLog(LogLevel::value::INFO, format, args...);
}

template<class... Args>
void Log::WARN(const char *format, Args... args) {
    addLog(LogLevel::value::INFO, format, args...);
}

template<class... Args>
void Log::ERROR(const char *format, Args... args) {
    addLog(LogLevel::value::INFO, format, args...);
}

template<class... Args>
void Log::FATAL(const char *format, Args... args) {
    addLog(LogLevel::value::INFO, format, args...);
}

#endif //TINYWEBSERVER_LOG_H
