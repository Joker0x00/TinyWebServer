//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_PARSEHTTPREQUEST_H
#define TINYWEBSERVER_PARSEHTTPREQUEST_H

#include <unordered_map>
#include <regex>
#include <unordered_set>
#include <mysql/mysql.h>
#include "../buffer/Buffer.h"
#include "../utils/Utils.h"
#include "../log/Log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

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
    std::string url_;
    std::string method_;
    std::string body_;
    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int>DEFAULT_HTML_TAG;
    std::unordered_map<std::string, std::string> post_;
public:

    ParseHttpRequest();
    ~ParseHttpRequest();
    void init();

    bool parse(Buffer &buf);
    void parse_url();
    bool parseRequestLine(const std::string &request_line);
    void parseRequestHeader(const std::string &header_line);
    void parseRequestBody(const std::string &body);


    std::string &method();
    std::string &version();
    std::string &path();

    bool keepAlive();

    void parsePost();

    void parseFromUrlencoded();

    static bool userVerify(const std::string &name, const std::string &pwd, bool isLogin);

    static int convertHex(char ch);
};


#endif //TINYWEBSERVER_PARSEHTTPREQUEST_H

//
//#ifndef HTTP_REQUEST_H
//#define HTTP_REQUEST_H
//
//#include <unordered_map>
//#include <unordered_set>
//#include <string>
//#include <regex>
//#include <errno.h>
//#include <mysql/mysql.h>  //mysql
//
//#include "../buffer/Buffer.h"
//#include "../log/Log.h"
//#include "../pool/sqlconnpool.h"
//#include "../pool/sqlconnRAII.h"
//
//class ParseHttpRequest {
//public:
//    enum PARSE_STATE {
//        REQUEST_LINE,
//        HEADERS,
//        BODY,
//        FINISH,
//    };
//
//    enum HTTP_CODE {
//        NO_REQUEST = 0,
//        GET_REQUEST,
//        BAD_REQUEST,
//        NO_RESOURSE,
//        FORBIDDENT_REQUEST,
//        FILE_REQUEST,
//        INTERNAL_ERROR,
//        CLOSED_CONNECTION,
//    };
//
//    ParseHttpRequest() { init(); }
//    ~ParseHttpRequest() = default;
//
//    void init();
//    bool parse(Buffer& buff);
//
//    std::string path() const;
//    std::string& path();
//    std::string method() const;
//    std::string version() const;
//    std::string GetPost(const std::string& key) const;
//    std::string GetPost(const char* key) const;
//
//    bool keepAlive() const;
//
//    /*
//    void HttpConn::ParseFormData() {}
//    void HttpConn::ParseJson() {}
//    */
//
//private:
//    bool ParseRequestLine_(const std::string& line);
//    void ParseHeader_(const std::string& line);
//    void ParseBody_(const std::string& line);
//
//    void ParsePath_();
//    void ParsePost_();
//    void ParseFromUrlencoded_();
//
//    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);
//
//    PARSE_STATE state_;
//    std::string method_, path_, version_, body_;
//    std::unordered_map<std::string, std::string> header_;
//    std::unordered_map<std::string, std::string> post_;
//
//    static const std::unordered_set<std::string> DEFAULT_HTML;
//    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
//    static int ConverHex(char ch);
//};
//
//
//#endif //HTTP_REQUEST_H