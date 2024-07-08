#include <iostream>
#include "server/Server.h"
int main() {
    Server("127.0.0.1", 20001, 3, 60000, LOG_TARGET_CONSOLE,
           LogLevel::value::DEBUG, 8, 65565,
           65535, 65535,
           "/home/wy/CLionProjects/cpp/TinyWebServer/").run();
    return 0;
}
