//
// Created by wy on 24-7-6.
//
#include "Router.h"
std::unordered_map<std::string, std::shared_ptr<Base>> Router::urlsToFunc = {
        {"/login", std::make_shared<Login>()}
};
std::shared_ptr<Response> Router::process(HttpParams &params) {
    auto processClass = urlsToFunc[params.url_];
    if (processClass->isNull()) {
        // 未找到处理该url的类
        return std::make_shared<Response>(404);
    }
    switch (params.method_) {
        case HttpMethod::GET:
            return processClass->GET(params);
        case HttpMethod::POST:
            return processClass->POST(params);
        case HttpMethod::PUT:
            return processClass->PUT(params);
            break;
        case HttpMethod::DELETE:
            return processClass->DELETE(params);
            break;
        default:
        {
            // 未找到该方法
            return std::make_shared<Response>(404);
        }
    }
}