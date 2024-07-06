//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_BASE_H
#define TINYWEBSERVER_BASE_H
#include "../http/HttpParams.h"
#include "../http/HttpResponse.h"

class Base {
public:
    virtual std::shared_ptr<Response> GET(HttpParams &params) = 0;
    virtual std::shared_ptr<Response> POST(HttpParams &params) = 0;
    virtual std::shared_ptr<Response> PUT(HttpParams &params) = 0;
    virtual std::shared_ptr<Response> DELETE(HttpParams &params) = 0;
    virtual bool isNull() {
        return true;
    }
};


#endif //TINYWEBSERVER_BASE_H
