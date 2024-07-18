//
// Created by wy on 24-7-3.
//
#include "ParseHttpRequest.h"
using namespace std;

const unordered_set<string> ParseHttpRequest::DEFAULT_HTML{
        "/index", "/register", "/login",
        "/welcome", "/video", "/picture", };

const unordered_map<string, int> ParseHttpRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},  };

void ParseHttpRequest::init() {
    method_ = url_ = version_ = body_ = "";
    state_ = PARSE_LINE;
    headers_.clear();
    post_.clear();
}

bool ParseHttpRequest::keepAlive() {
    if(headers_.count("Connection") == 1) {
        return headers_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool ParseHttpRequest::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if(buff.getContentLen() <= 0) {
        return false;
    }
    while(buff.getContentLen() && state_ != FINISH) {
        const char* lineEnd = search(buff.getReadPtr(), buff.getWritePtr(), CRLF, CRLF + 2);
        std::string line(buff.getConstReadPtr(), lineEnd);
        switch(state_)
        {
            case PARSE_LINE:
                if(!parseRequestLine(line)) {
                    return false;
                }
                parse_url();
                break;
            case PARSE_HEADERS:
                parseRequestHeader(line);
                if(buff.getContentLen() <= 2) {
                    state_ = FINISH;
                }
                break;
            case PARSE_BODY:
                parseRequestBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.getWritePtr()) { break; }
        buff.addReadIdxUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), url_.c_str(), version_.c_str());
    return true;
}

void ParseHttpRequest::parse_url() {
    if(url_ == "/") {
        url_ = "/index.html";
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == url_) {
                url_ += ".html";
                break;
            }
        }
    }
}

bool ParseHttpRequest::parseRequestLine(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        method_ = subMatch[1];
        url_ = subMatch[2];
        version_ = subMatch[3];
        state_ = PARSE_HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void ParseHttpRequest::parseRequestHeader(const string& line) {
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        headers_[subMatch[1]] = subMatch[2];
    }
    else {
        state_ = PARSE_BODY;
    }
}

void ParseHttpRequest::parseRequestBody(const string& line) {
    body_ = line;
    parsePost();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int ParseHttpRequest::convertHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void ParseHttpRequest::parsePost() {
    if(method_ == "POST" && headers_["Content-Type"] == "application/x-www-form-urlencoded") {
        parseFromUrlencoded();
        if(DEFAULT_HTML_TAG.count(url_)) {
            int tag = DEFAULT_HTML_TAG.find(url_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(userVerify(post_["username"], post_["password"], isLogin)) {
                    url_ = "/welcome.html";
                }
                else {
                    url_ = "/error.html";
                }
            }
        }
    }
}

void ParseHttpRequest::parseFromUrlencoded() {
    if(body_.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = convertHex(body_[i + 1]) * 16 + convertHex(body_[i + 2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool ParseHttpRequest::userVerify(const string &name, const string &pwd, bool isLogin) {
    if(name.empty() || pwd.empty()) { return false; }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII give_me_a_name(&sql,  SqlConnPool::Instance());
    assert(sql);

    bool flag = false;
    char order[256] = { 0 };

    MYSQL_RES *res = nullptr;

    if(!isLogin) { flag = true; }
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    mysql_num_fields(res);
    mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);
        /* 注册行为 且 用户名未被使用*/
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        }
        else {
            flag = false;
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order)
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Insert error!");
            flag = false;
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}


std::string &ParseHttpRequest::path(){
    return url_;
}
std::string &ParseHttpRequest::method() {
    return method_;
}

std::string &ParseHttpRequest::version() {
    return version_;
}

ParseHttpRequest::ParseHttpRequest() {
    init();
}

ParseHttpRequest::~ParseHttpRequest() {

}
