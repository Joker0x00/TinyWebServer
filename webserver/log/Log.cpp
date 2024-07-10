//
// Created by wy on 24-7-2.
//

#include "Log.h"

Log::Log() {
    saveDir_ = nullptr;
    filename_ = nullptr;
    suffix_ = nullptr;
    log_Queue_ = nullptr;
    workThread_ = nullptr;
    isRun_ = true;
    fp_ = nullptr;
    target_ = LOG_TARGET_CONSOLE;
    logCnt = 0;
}

Log::~Log() {
    printf("close logging...\n");
    if (log_Queue_->size()) {
        sleep(2);
    }
    isRun_ = false;
    fflush(fp_);
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }
    delete[] filename_;
}



Log* Log::getInstance() {
    static Log log_;
    return &log_;
}

void
Log::init(LogTarget target, const char *save_dir, const char *suffix, LogLevel::value logLevel,
          size_t maxQueueSize) {
    saveDir_ = save_dir;
    suffix_ = suffix;
    logLevel_ = logLevel;
    filename_ = new char(MAX_FILENAME_LEN);
    target_ = target;
    if (maxQueueSize > 0) {
        isAsync_ = true;
        if (!log_Queue_) {
            std::unique_ptr<LogQueue<std::string>> q(new LogQueue<std::string>(maxQueueSize));
            log_Queue_ = std::move(q);
            std::unique_ptr<std::thread> t(new std::thread(asyncWriteLogThread));
            workThread_ = std::move(t);
        }
    } else {
        isAsync_ = false;
    }
    if (!initLogFile()) {
        printf("start loging failed...\n");
        return ;
    }
}



void Log::asyncWriteLogThread() {
    Log::getInstance()->AsyncWrite_();
}

bool Log::initLogFile() {
    if (target_ == LOG_TARGET_CONSOLE) {
        fp_ = stdout;
    } else if (target_ == LOG_TARGET_FILE){
        char time_str[25];
        util::Date::getDateTimeByFormat(time_str, 25, "%Y_%m_%d_%H_%M_%S");
        snprintf(filename_, MAX_FILENAME_LEN, "%s%s", time_str, suffix_);
        fp_ = util::File::createFile(saveDir_, filename_);
        if (fp_ == nullptr) {
            return false;
        }
    } else {
        fp_ = nullptr;
        return true;
    }
    printf("start logging...\n");
    return true;
}

void Log::setEntryTime() {
    char time_str[25];
    util::Date::getDateTime(time_str, 25);
    size_t str_len = strlen(time_str);
    time_str[str_len] = ' ';
    buf_.append(time_str, str_len + 1); // 追加空格
}

void Log::setEntryType(LogLevel::value t) {
     buf_.append(LogLevel::toString(t) + std::string(" "));
}

void Log::setEntryMsg(const std::string &msg) {
    buf_.append(msg + std::string("\n"));
}

void Log::appendEntry(const std::string &entry) {
    log_Queue_->push(entry);
}

void Log::flush() {
    if (isAsync_) {
        log_Queue_->flush();
    }
    fflush(fp_);
}

void Log::addLog(LogLevel::value type, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // 向buf_中添加数据，如果多线程访问需要确保只有一个线程访问buf_
    std::unique_lock<std::mutex> locker(mtx_);
    int n = snprintf(buf_.getWritePtr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                     t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                     t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
    buf_.addWriteIdx(n);
    setEntryType(type);

    va_start(vaList, format);
    int m = vsnprintf(buf_.getWritePtr(), buf_.getLeftLen(), format, vaList);
    va_end(vaList);
    buf_.addWriteIdx(m);
    buf_.append("\n\0", 2);

    if (isAsync_ && log_Queue_ && !log_Queue_->full()) {
        log_Queue_->push(buf_.getStringAndReset());
    } else {
        fputs(buf_.getReadPtr(), fp_); // 这一部分不确定作用
    }
}

void Log::AsyncWrite_() {
    std::string str;
    while (log_Queue_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}






