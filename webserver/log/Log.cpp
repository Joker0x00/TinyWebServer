//
// Created by wy on 24-7-2.
//

#include "Log.h"

Log::~Log() {
    printf("close logging...\n");
    if (log_Queue_->size()) {
        sleep(2);
    }
    isRun = false;
    fflush(fp_);
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }
    delete[] filename_;
}

Log::Log() {
    saveDir_ = nullptr;
    filename_ = nullptr;
    suffix_ = nullptr;
    log_Queue_ = nullptr;
    workThread_ = nullptr;
    isRun = true;
    fp_ = nullptr;
    target_ = LOG_TARGET_CONSOLE;
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
    if (!log_Queue_) {
        std::unique_ptr<LogQueue<std::string>> q(new LogQueue<std::string>(maxQueueSize));
        log_Queue_ = std::move(q);
        std::unique_ptr<std::thread> t(new std::thread(Log::asyncWriteLogThread));
        workThread_ = std::move(t);
    }
    if (!initLogFile()) {
        printf("start loging failed...\n");
        return ;
    }
}

void Log::asyncWriteLogThread() {
    auto l = getInstance();
    while(l->isRun) {
        auto entry = l->log_Queue_->pop();
        l->writeFile(entry);
    }
}

bool Log::initLogFile() {
    if (target_ == LOG_TARGET_CONSOLE) {
        fp_ = stdout;
    } else {
        char time_str[25];
        util::Date::getDateTimeByFormat(time_str, 25, "%Y_%m_%d_%H_%M_%S");
        snprintf(filename_, MAX_FILENAME_LEN, "%s%s", time_str, suffix_);
        puts(filename_);
        fp_ = util::File::createFile(saveDir_, filename_);
        if (fp_ == nullptr) {
            return false;
        }
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

void Log::writeFile(const std::string &data) {
    fprintf(fp_,"%s", data.c_str());
    fflush(fp_);
}







