#include <iostream>
#include "server/Server.h"
int main() {
    Server("127.0.0.1", 20001, 3, 60000, LOG_TARGET_FILE,  /* ip地址 端口 触发模式 超时事件 日志输出位置 */
           LogLevel::value::DEBUG, 6, 65535,                /* 日志等级 线程数 定时器队列长度 */
           65535, 65535 /* 最大用户数 事件数组长度 */
           ).run(); /* 日志存储路径 */
    return 0;
}
