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
              LogTarget logTarget, size_t maxQueueSize =  1024); // 初始化日志系统
    static void asyncWriteLogThread(); // 工作线程将日志异步写入文件的函数
    bool initLogFile(); // 初始化日志文件
    void setEntryTime(); // 设置日志条目时间头
    void setEntryType(LogLevel::value t); // 设置日志条目类型
    void setEntryMsg(const std::string &msg);
    template <class ...Args>
    void addLog(LogLevel::value type, const char *format, Args... args);
    void appendEntry(const std::string& entry);
    void writeFile(const std::string &data);


    static Log* getInstance();

private:
    Log();
    ~Log();
};

template <class ...Args>
void Log::addLog(LogLevel::value type, const char *format, Args... args) {
    if (type < logLevel_) return; // 过滤日志
    std::string message;
    util::String::formatPrintStr(message, format, args...);
    setEntryType(type);
    setEntryTime();
    setEntryMsg(message);
    auto entry = buf_.getStringAndReset();
    appendEntry(entry);
}

#endif //TINYWEBSERVER_LOG_H
