#include <iostream>
#include "server/Server.h"
int main() {
    Server("127.0.0.1", 20001, 3, 60000,
           LogLevel::value::DEBUG, 8, 4096,
           65535, 4096,
           "/home/wy/CLionProjects/cpp/TinyWebServer/").run();
    return 0;
}
