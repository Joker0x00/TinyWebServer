//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_LOGIN_H
#define TINYWEBSERVER_LOGIN_H
#include "Base.h"
#include "../http/HttpParams.h"
#include "../http/HttpResponse.h"
#include "../lib/json/json.h"
// 登录接口
class Login: public Base{
public:
    std::string GET(HttpParams &params) override {
        Log::INFO("%s", "/login, GET");
        return Response::getResponse(200, "success");
    }

    std::string POST(HttpParams &params) override {
        // 此处验证参数
        printf("%s\n", params.body_.c_str());
        Json::Reader reader;
        Json::Value data;
        if (!reader.parse(params.body_, data)) {
            return Response::getResponse(400, "json parse failed");
        }
        std::string username = data.get("username", "").asString();
        std::string password = data.get("password", "").asString();
        if (username.empty() || password.empty()) {
            return Response::getResponse(200, "params is null");
        }
        if (!(username == "wy" && password == "123456")) {
            return Response::getResponse(200, "username or password is error");
        }
        return Response::getResponse(200, "success");
    }

    std::string PUT(HttpParams &params) override {

    }

    std::string DELETE(HttpParams &params) override {

    }

    bool isNull() override {
        return false;
    }
};


#endif //TINYWEBSERVER_LOGIN_H