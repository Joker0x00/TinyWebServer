//
// Created by wy on 24-7-3.
//

#include "ParseHttpRequest.h"


bool ParseHttpRequest::parseRequestLine(const std::string &request_line) {
    std::regex requestLinePattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch matches;
    if (std::regex_match(request_line, matches, requestLinePattern)) {
        if (matches.size() == 4) {
            method_ = matches[1];
            url_ = matches[2];
            if (!parse_url(&parsedUrl_, url_))
                return false;
            version_ = matches[3];
            state_ = PARSE_HEADERS; // 改变当前状态
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

bool ParseHttpRequest::parseRequestHeader(const std::string &header_line) {
    std::regex header_pattern(R"(^([^:]*):(.*)$)");
    std::smatch matches;
    if (std::regex_match(header_line, matches, header_pattern)) {
        if (matches.size() == 3) {
            if (!headers_.count(matches[1])) {
                headers_[matches[1]] = matches[2];
            }
            return true;
        }
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
    headers_.clear();
}

ParseHttpRequest::~ParseHttpRequest() {
    headers_.clear();
}

bool ParseHttpRequest::parse(Buffer &buf) {
    // 执行状态机，解析HTTP请求
    while(state_ != FINISH) {
        std::string line;
        size_t line_len;
        if (state_ != PARSE_BODY) {
            const char *line_end = util::String::findCRLF(buf.getReadPtr(), buf.getWritePtr());
            if (line_end == nullptr)
                return false;
            line_len = line_end - buf.getReadPtr();
            line = std::string(buf.getReadPtr(), line_len);
            buf.addReadIdxUntil(line_end + 2);
        } else {
            line = buf.getStringAndReset();
        }
        switch (state_) {
            case PARSE_LINE:
                if (!parseRequestLine(line)) {
                    return false;
                }
                break;
            case PARSE_HEADERS:
                if (line_len == 0) {
                    // 请求头已解析完毕
                    state_ = PARSE_BODY;
                } else {
                    parseRequestHeader(line);
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

bool ParseHttpRequest::keepAlive() {
    if (headers_.count("Connection")) {
        if (headers_["Connection"] ==  "keep-alive") {
            return true;
        }
    }
    return false;
}

void ParseHttpRequest::init() {
    state_ = PARSE_LINE;
    method_ = url_ = version_ = data_ = "";
    headers_.clear();
}

HttpParams ParseHttpRequest::getParams() {
    return {method_, data_, parsedUrl_.path, parsedUrl_.queryParams};
}

