//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPMETHOD_H
#define TINYWEBSERVER_HTTPMETHOD_H

#include <string>

class HttpMethod {
public:
    enum method {
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE
    };
    static std::string toStr(method value) {
        switch (value) {
            case GET:
                return "GET";
            case HEAD:
                return "HEAD";
            case POST:
                return "POST";
            case PUT:
                return "PUT";
            case DELETE:
                return "DELETE";
            case CONNECT:
                return "CONNECT";
            case OPTIONS:
                return "OPTIONS";
            case TRACE:
                return "TRACE";
            default:
                return "";
        }
    }
};
#endif //TINYWEBSERVER_HTTPMETHOD_H
