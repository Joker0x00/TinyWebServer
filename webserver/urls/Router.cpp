//
// Created by wy on 24-7-6.
//
#include "Router.h"
std::unordered_map<std::string, std::shared_ptr<Base>> Router::urlsToFunc = {
        {"/login", std::make_shared<Login>()}
};
std::string Router::resource_path;

pis Router::process(HttpParams &params) {
    auto processClass = urlsToFunc[params.url_];
    if (processClass == nullptr) {
        // 未找到处理该url的类
        LOG_WARN("%s Not Found", params.url_.c_str());
        return {1, "404.html"};
    }
    LOG_INFO("%s %s", HttpMethod::toStr(params.method_).c_str(), params.url_.c_str());
    pis res;
    switch (params.method_) {
        case HttpMethod::GET:
            res = std::move(processClass->GET(params));
            break;
        case HttpMethod::POST:
            res = std::move(processClass->POST(params));
            break;
        case HttpMethod::PUT:
            res = std::move(processClass->PUT(params));
            break;
        case HttpMethod::DELETE:
            res = std::move(processClass->DELETE(params));
            break;
        default:
        {
            // 未找到该方法
            return {1, "404.html"};
        }
    }
    //  验证访问的html是否存在
    if (res.first) {
        std::ifstream file(resource_path);
        if (!file) {
            return {1, "404.html"};
        }
    }
    return res;
}