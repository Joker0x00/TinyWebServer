//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_PARSEHTTPREQUEST_H
#define TINYWEBSERVER_PARSEHTTPREQUEST_H

#include <unordered_map>
#include <regex>

#include "../buffer/Buffer.h"
#include "../utils/Utils.h"
#include "../log/Log.h"
#include "HttpMethod.h"
#include "HttpParams.h"
#include "HttpParams.h"

class ParseHttpRequest {
public:
    // 当前的处理状态
    enum Status {
        PARSE_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        FINISH
    };
private:
    std::string version_; // HTTP版本
    std::unordered_map<std::string, std::string> headers_; // 头部字段
    Status state_ = PARSE_LINE;
    HttpParams params;
public:

    ParseHttpRequest();
    ~ParseHttpRequest();
    void init();

    bool parse(Buffer &buf);
    void parse_url();
    bool parseRequestLine(const std::string &request_line);
    bool parseRequestHeader(const std::string &header_line);
    bool parseRequestBody(const std::string &body);

    HttpParams &getParams();


    HttpMethod::MethodType &getMethod();
    std::string &getVersion();
    std::unordered_map<std::string, std::string> &getHeaders();
    std::string &getBody();
    bool keepAlive();
};


#endif //TINYWEBSERVER_PARSEHTTPREQUEST_H
