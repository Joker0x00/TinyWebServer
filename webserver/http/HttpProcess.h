//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPPROCESS_H
#define TINYWEBSERVER_HTTPPROCESS_H

#include "HttpResponse.h"
#include "ParseHttpRequest.h"
#include "HttpParams.h"
// Http业务逻辑处理
class HttpProcess {
public:
    HttpProcess();
    ~HttpProcess();
    static void process() {

    }
};


#endif //TINYWEBSERVER_HTTPPROCESS_H
