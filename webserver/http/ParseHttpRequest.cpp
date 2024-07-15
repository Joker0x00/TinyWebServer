//
// Created by wy on 24-7-3.
//

#include "ParseHttpRequest.h"

const std::unordered_set<std::string> ParseHttpRequest::DEFAULT_HTML{
        "/index", "/register", "/login",
        "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> ParseHttpRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},  };

// 初始化request，重置内部参数
void ParseHttpRequest::init() {
    state_ = PARSE_LINE;
    params.url_ = version_ = params.body_ = "";
    params.method_ = HttpMethod::NONE;
    headers_.clear();
}
// 解析请求行
bool ParseHttpRequest::parseRequestLine(const std::string &request_line) {
    std::regex requestLinePattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch matches;
    if (std::regex_match(request_line, matches, requestLinePattern)) {
        if (matches.size() == 4) {
            params.method_ = HttpMethod::toMethod[matches[1]];
            params.url_ = matches[2];
            version_ = matches[3];
            state_ = PARSE_HEADERS; // 改变当前状态
            return true;
        }
    }
    LOG_ERROR("Parse Request Error: %s", request_line.c_str());
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
    params.body_ = body;
    state_ = FINISH;
    return true;
}

ParseHttpRequest::ParseHttpRequest() {
    init();
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
                parse_url();
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

//bool ParseHttpRequest::parse_url(const std::string &url) {
//    std::regex urlPattern(R"(^([^\?#]*)(?:\?([^#]*))?)");
//    std::smatch matches;
//
//    if (std::regex_match(url, matches, urlPattern)) {
//        std::string queryString = matches[2].str();
//        std::regex queryPattern(R"(([^&=]+)=([^&=]*)(&|$))");
//        std::sregex_iterator it(queryString.begin(), queryString.end(), queryPattern);
//        std::sregex_iterator end;
//        for (; it != end; ++it) {
//            params.urlParams_[it->str(1)] = it->str(2);
//        }
//    } else {
//        return false;
//    }
//
//    return true;
//}

void ParseHttpRequest::parse_url() {
    if(params.url_ == "/") {
        params.url_ = "/index.html";
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == params.url_) {
                params.url_ += ".html";
                break;
            }
        }
    }
}

HttpMethod::MethodType &ParseHttpRequest::getMethod() {
    return params.method_;
}

std::string &ParseHttpRequest::getVersion() {
    return version_;
}

std::unordered_map<std::string, std::string> &ParseHttpRequest::getHeaders() {
    return headers_;
}

std::string &ParseHttpRequest::getBody() {
    return params.body_;
}
// 返回当前http请求是否keep alive
bool ParseHttpRequest::keepAlive() {
    if (headers_.count("Connection")) {
        return headers_["Connection"] ==  "keep-alive" && version_ == "1.1";
    }
    return false;
}

HttpParams &ParseHttpRequest::getParams() {
    return params;
}

