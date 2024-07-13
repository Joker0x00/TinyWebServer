//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_HTTPRESPONSE_H
#define TINYWEBSERVER_HTTPRESPONSE_H
#include <unordered_map>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../utils/Utils.h"
#include "../buffer/Buffer.h"
#include "../log/Log.h"
#include "Response.h"
#include "HttpMethod.h"


// 构造HTTP响应报文
class HttpResponse {
private:
    static std::unordered_map<int, std::string> CODE;
public:
    HttpResponse() = default;
    ~HttpResponse() = default;

    static void makeResponse(HttpMethod::MethodType &method, int code, std::string&& res, bool keepAlive, Buffer &buf);

    static void addResponseLine(int code, const std::string &title, Buffer &buf);

    static void addHeaders(bool keepAlive, Buffer &buf);

    static void addBody(const std::string &&data, Buffer &buf);

    static void addCors(Buffer &buf);
};


#endif //TINYWEBSERVER_HTTPRESPONSE_H
