//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_BASE_H
#define TINYWEBSERVER_BASE_H
#include "../http/HttpParams.h"
#include "../http/HttpResponse.h"

class Base {
public:
    virtual std::string GET(HttpParams &params) { return ""; }
    virtual std::string POST(HttpParams &params) { return ""; }
    virtual std::string PUT(HttpParams &params) { return ""; }
    virtual std::string DELETE(HttpParams &params) { return ""; }
    virtual std::string OPTIONS(HttpParams &params) {
        return "";
    }
    virtual bool isNull() {
        return true;
    }
};


#endif //TINYWEBSERVER_BASE_H
