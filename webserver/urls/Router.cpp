//
// Created by wy on 24-7-6.
//
#include "Router.h"
std::unordered_map<std::string, std::shared_ptr<Base>> Router::urlsToFunc = {
        {"/login", std::make_shared<Login>()}
};
std::string Router::process(HttpParams &params) {
    auto processClass = urlsToFunc[params.url_];
    if (processClass == nullptr) {
        // 未找到处理该url的类
        Log::WARN("%s Not Found", params.url_.c_str());
        return Response::getResponse(404, "Not Found");
    }
    Log::INFO("%s %s", params.url_.c_str(), HttpMethod::toStr(params.method_).c_str());
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
            return Response::getResponse(404, "error");
        }
    }
}