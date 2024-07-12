//
// Created by wy on 24-7-6.
//
#include "HttpParams.h"
#include "HttpMethod.h"
HttpParams::HttpParams(const std::string& method, std::string body, std::string url,
           std::unordered_map<std::string, std::string> &urlParams) : method_(HttpMethod::toMethod[method]), body_(std::move(body)),
           urlParams_(urlParams), url_(std::move(url)) {
}
