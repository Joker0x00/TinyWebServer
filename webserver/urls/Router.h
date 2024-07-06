//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_ROUTER_H
#define TINYWEBSERVER_ROUTER_H

#include <unordered_map>
#include <memory>
#include "../views/Base.h"
#include "../views/Login.h"
#include "../http/HttpParams.h"
#include "../log/Log.h"
#include "../http/Response.h"

class Router {
public:
    static std::unordered_map<std::string, std::shared_ptr<Base>> urlsToFunc; // urls映射
    // 分发并处理请求
    static std::shared_ptr<Response> process(HttpParams &params);
};

#endif //TINYWEBSERVER_ROUTER_H