//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_PARSEHTTPREQUEST_H
#define TINYWEBSERVER_PARSEHTTPREQUEST_H

#include <unordered_map>
#include <regex>

#include "../buffer/Buffer.h"
#include "../utils/Utils.h"
#include "ParsedUrl.h"
#include "../log/Log.h"

enum Status {
    PARSE_LINE,
    PARSE_HEADERS,
    PARSE_BODY,
    FINISH
};
class ParseHttpRequest {
public:

private:
    std::string method_; // 方法
    std::string url_; // 请求url
    std::string version_; // HTTP版本
    std::unordered_map<std::string, std::string> headers_; // 头部字段
    std::string data_; // 请求体
    Status state_;
    ParsedUrl parsedUrl_;
public:

    ParseHttpRequest();
    ~ParseHttpRequest();
    void init();

    bool parse(Buffer &buf);
    static bool parse_url(ParsedUrl *parsedURL, const std::string& url);
    bool parseRequestLine(const std::string &request_line);
    bool parseRequestHeader(const std::string &header_line);
    bool parseRequestBody(const std::string &body);

    std::string &getMethod();
    ParsedUrl *getParsedUrl_();
    std::string &getVersion();
    std::unordered_map<std::string, std::string> &getHeaders();
    std::string &getBody();
    bool keepAlive();
};


#endif //TINYWEBSERVER_PARSEHTTPREQUEST_H
