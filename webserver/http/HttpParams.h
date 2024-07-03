//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPPARAMS_H
#define TINYWEBSERVER_HTTPPARAMS_H
#include <string>
#include <unordered_map>
#include <utility>

class HttpParams {
public:
    const std::string method;
    const std::string body;
    const std::unordered_map<std::string, std::string> urlParams;

    HttpParams(std::string &method, std::string &body,
               const std::unordered_map<std::string, std::string> &urlParams) : method(std::move(method)), body(std::move(body)),
                                                                                urlParams(urlParams) {}
    HttpParams()=default;
    ~HttpParams()=default;
};


#endif //TINYWEBSERVER_HTTPPARAMS_H
