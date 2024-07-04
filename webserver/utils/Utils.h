//
// Created by wy on 24-7-2.
//

#ifndef TINYWEBSERVER_UTILS_H
#define TINYWEBSERVER_UTILS_H

#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace  util {
    class Date {
    public:
        static size_t now() {
            return static_cast<size_t>(time(nullptr));
        }

        static void getDateTime(char* t_str, size_t len)
        {
            assert (t_str);
            time_t t;
            time (&t);
            strftime(t_str, len, "%Y-%m-%d %H:%M:%S",localtime(&t));
        }

        static void getDateTimeByFormat(char* t_str, size_t len, const char* format) {
            assert (t_str);
            time_t t;
            time (&t);
            strftime(t_str, len, format,localtime(&t));
        }
    };
    class String {
    public:
        // 泛型
        template <typename... Args>
        static void formatPrintStr(std::string &res, const char *pformat, Args... args)
        {
            // 计算字符串长度
            int len_str = std::snprintf(nullptr, 0, pformat, args...);

            if (len_str <= 0) {
                res = "";
                return;
            }

            len_str++;
            char *str_out  = new char[len_str];

            // 构造字符串
            std::snprintf(str_out, len_str, pformat, args...);
            // 保存构造结果
            res = std::string(str_out);
            // 释放空间
            delete[] str_out;
            str_out = nullptr;
        }
        static const char* findCRLF(const char* buffer, const char* buffer_end) {
            const char* target = "\r\n";
//            std::cout << std::string(buffer, buffer_end) << std::endl;
            // 使用 std::search 查找子字符串
            const char* result = std::search(buffer, buffer_end, target, target + 2);
            if (result != buffer_end) {
                return result; // 找到并返回指向 '\r\n' 的指针
            }

            return nullptr; // 未找到
        }
    };
    class File {
    public:
        // 检查路径是否存在
        static bool pathExists(const char* path) {
            struct stat info;
            return stat(path, &info) == 0;
        }

        // 创建日志文件并返回文件指针
        static FILE* createFile(const char* directory, const char* filename) {
            if (!pathExists(directory)) {
                fprintf(stderr, "Directory does not exist: %s\n", directory);
                return nullptr;
            }

            // 构造完整的文件路径
            std::string filepath = std::string(directory) + "/" + filename;
            FILE* file = fopen(filepath.c_str(), "w");
            if (file == nullptr) {
                fprintf(stderr, "Failed to create file: %s\n", strerror(errno));
            }
            return file;
        }
    };
}

#endif //TINYWEBSERVER_UTILS_H

