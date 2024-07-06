//
// Created by wy on 24-7-6.
//
#include "HttpParams.h"
#include "HttpMethod.h"
HttpParams::HttpParams(const std::string& method, std::string body, std::string url,
           std::unordered_map<std::string, std::string> &urlParams) : body_(std::move(body)), url_(std::move(url)),
           urlParams_(urlParams) {
    method_ = HttpMethod::toMethod[method];
//    method_ = HttpMethod::MethodType::GET;
}
