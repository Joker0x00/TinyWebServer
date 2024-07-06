//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPPARAMS_H
#define TINYWEBSERVER_HTTPPARAMS_H
#include <string>
#include <unordered_map>
#include <utility>
#include "HttpMethod.h"
class HttpParams {
public:
    HttpMethod::MethodType method_=HttpMethod::MethodType::NONE;
    std::string body_; // 请求体 通常是json格式
    std::unordered_map<std::string, std::string> urlParams_; // url中的参数
    std::string url_;
    HttpParams(const std::string& method, std::string body, std::string url,
               std::unordered_map<std::string, std::string> &urlParams);
    ~HttpParams()=default;
};


#endif //TINYWEBSERVER_HTTPPARAMS_H
