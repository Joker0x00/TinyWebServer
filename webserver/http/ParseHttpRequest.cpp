//
// Created by wy on 24-7-3.
//

#include "ParseHttpRequest.h"
// 初始化request，重置内部参数
void ParseHttpRequest::init() {
    state_ = PARSE_LINE;
    method_ = url_ = version_ = data_ = "";
    headers_.clear();
}
// 解析请求行
bool ParseHttpRequest::parseRequestLine(const std::string &request_line) {
    std::regex requestLinePattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch matches;
    if (std::regex_match(request_line, matches, requestLinePattern)) {
        if (matches.size() == 4) {
            method_ = matches[1];
            url_ = matches[2];
            version_ = matches[3];
            state_ = PARSE_HEADERS; // 改变当前状态
            return true;
        }
    }
    LOG_ERROR("Parse Request Error");
    return false;
}
// 解析请求头
bool ParseHttpRequest::parseRequestHeader(const std::string &header_line) {
    std::regex header_pattern(R"(^([^:]+):\s*(.*)$)");
    std::smatch matches;
    if (std::regex_match(header_line, matches, header_pattern)) {
        if (matches.size() == 3) {
            headers_[matches[1]] = matches[2];
            return true;
        }
    } else {
        state_ = PARSE_BODY;
    }
    return false;
}
// 解析带有请求体的请求 POST PUT
bool ParseHttpRequest::parseRequestBody(const std::string &body) {
    data_ = body;
    state_ = FINISH;
    return true;
}

ParseHttpRequest::ParseHttpRequest() {
    state_ = PARSE_LINE;
    method_ = url_ = version_ = data_ = "";
}

ParseHttpRequest::~ParseHttpRequest() {
    headers_.clear();
}

bool ParseHttpRequest::parse(Buffer &buf) {
    if (buf.getContentLen() <= 0) {
        return false;
    }
    const char CRLF[] = "\r\n";
    // 执行状态机，解析HTTP请求
    while(buf.getContentLen() && state_ != FINISH) {
        std::string line;
        size_t line_len = 0;
        if (state_ != PARSE_BODY) {
            const char* line_end = std::search(buf.getReadPtr(), buf.getWritePtr(), CRLF, CRLF + 2);
            if (line_end == buf.getWritePtr()) // 查找不到回车换行符
                return false;
            line_len = line_end - buf.getReadPtr();
            line = std::string(buf.getReadPtr(), line_len);
            // 移动读指针到"\r\n"的下一个位置
            buf.addReadIdxUntil(line_end + 2);
        } else {
            line = buf.getStringAndReset();
        }
        switch (state_) {
            case PARSE_LINE:
                if (!parseRequestLine(line)) {
                    return false;
                }
                parse_url(&parsedUrl_, url_);
                break;
            case PARSE_HEADERS:
                if (line_len == 0) {
                    // 请求头已解析完毕
                    state_ = PARSE_BODY;
                } else {
                    parseRequestHeader(line);
                    if (buf.getContentLen() <= 2) {
                        state_ = FINISH;
                    }
                }
                break;
            case PARSE_BODY:
                parseRequestBody(line);
                break;
            case FINISH:
                break;
        }
    }
    return true;
}

bool ParseHttpRequest::parse_url(ParsedUrl *parsedURL, const std::string &url) {
    std::regex urlPattern(R"(^([^\?#]*)(?:\?([^#]*))?)");
    std::smatch matches;

    if (std::regex_match(url, matches, urlPattern)) {
        parsedURL->path = matches[1].str().empty() ? "/" : matches[1].str();
        std::string queryString = matches[2].str();
        std::regex queryPattern(R"(([^&=]+)=([^&=]*)(&|$))");
        std::sregex_iterator it(queryString.begin(), queryString.end(), queryPattern);
        std::sregex_iterator end;
        for (; it != end; ++it) {
            parsedURL->queryParams[it->str(1)] = it->str(2);
        }
    } else {
        return false;
    }

    return true;
}

std::string &ParseHttpRequest::getMethod() {
    return method_;
}

ParsedUrl *ParseHttpRequest::getParsedUrl_() {
    return &parsedUrl_;
}

std::string &ParseHttpRequest::getVersion() {
    return version_;
}

std::unordered_map<std::string, std::string> &ParseHttpRequest::getHeaders() {
    return headers_;
}

std::string &ParseHttpRequest::getBody() {
    return data_;
}
// 返回当前http请求是否keep alive
bool ParseHttpRequest::keepAlive() {
    if (headers_.count("Connection")) {
        return headers_["Connection"] ==  "keep-alive" && version_ == "1.1";
    }
    return false;
}



HttpParams ParseHttpRequest::getParams() {
    return {method_, data_, parsedUrl_.path, parsedUrl_.queryParams};
}

