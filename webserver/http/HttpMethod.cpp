//
// Created by wy on 24-7-6.
//
#include "HttpMethod.h"
// 如果key不存在，则默认会使用第一个变量，NONE
// 大问题，这里忘记加作用域了，一直显示未定义
std::unordered_map<std::string, HttpMethod::MethodType> HttpMethod::toMethod = {
        {"GET", HttpMethod::GET},
        {"POST", HttpMethod::POST},
        {"PUT", HttpMethod::PUT},
        {"GET", HttpMethod::DELETE},
};