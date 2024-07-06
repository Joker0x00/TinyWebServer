//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_LOGIN_H
#define TINYWEBSERVER_LOGIN_H
#include "Base.h"
#include "../http/HttpParams.h"
#include "../http/HttpResponse.h"
// 登录接口
class Login: public Base{
public:
    std::shared_ptr<Response> GET(HttpParams &params) override {
        printf("处理get请求");
        return std::make_shared<Response>(200);
    }

    std::shared_ptr<Response> POST(HttpParams &params) override {

    }

    std::shared_ptr<Response> PUT(HttpParams &params) override {

    }

    std::shared_ptr<Response> DELETE(HttpParams &params) override {

    }

    bool isNull() override {
        return false;
    }
};


#endif //TINYWEBSERVER_LOGIN_H
