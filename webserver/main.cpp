#include <iostream>
#include "server/Server.h"
#include "lib/inih-r58/ini.h"

// 用于存储配置的全局变量
std::map<std::string, std::string> config;

// 解析配置文件的回调函数
static int handler(void* user, const char* section, const char* name, const char* value)
{
    std::string key = std::string(section) + "." + name;
    config[key] = value;
    return 1;
}

int main() {

    // 读取配置文件
    if (ini_parse("config.ini", handler, nullptr) < 0) {
        std::cerr << "Can't load 'config.ini'\n";
        return 1;
    }

    Server(config["server.host"].c_str(), stoi(config["server.port"]),
            stoi(config["server.trigMode"]),
            stoi(config["server.timeout"]),
           static_cast<LogTarget>(stoi(config["log.logTarget"])),
           static_cast<LogLevel::value>(stoi(config["log.logLevel"])),
            stoi(config["server.max_thread_cnt"]),
            stoi(config["server.max_timer_cnt"]),
            stoi(config["server.max_fd"]),
            stoi(config["server.max_epoll_events"]),
            stoi(config["database.port"]),
            config["database.user"].c_str(),
            config["database.password"].c_str(),
            config["database.dbName"].c_str(),
            stoi(config["database.connPoolNum"])
           ).run();
    return 0;
}
