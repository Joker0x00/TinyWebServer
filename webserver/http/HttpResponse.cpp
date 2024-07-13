//
// Created by wy on 24-7-3.
//

#include "HttpResponse.h"

std::unordered_map<int, std::string> HttpResponse::CODE = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};

void HttpResponse::addResponseLine(int code, const std::string &title, Buffer &buf) {
    std::string res;
    util::String::formatPrintStr(res, "%s %d %s\r\n", "HTTP/1.1", code, title.c_str());
    buf.append(res);
}

void HttpResponse::addHeaders(bool keepAlive, Buffer &buf) {
    buf.append("Connection: ");
    if (keepAlive) {
        buf.append("keep-alive\r\n");
        buf.append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buf.append("close\r\n");
    }
    buf.append("Content-Type:application/json\r\n");
    buf.append("Access-Control-Allow-Origin:*\r\n");
}

void HttpResponse::addCors(Buffer &buf) {
    buf.append("Access-Control-Allow-Methods:POST, OPTIONS, GET, PUT, DELETE\r\n");
    buf.append("Access-Control-Allow-Headers:Content-Type, Connection, Content-Length, Keep-Alive, \r\n");
    buf.append("Access-Control-Max-Age:3600\r\n");
    buf.append("Cache-Control:no-cache, no-store, must-revalidate\r\n");
}

void HttpResponse::addBody(const std::string &&data, Buffer &buf) {
    buf.append("Content-Length: " + std::to_string(data.length()) + "\r\n\r\n");
    buf.append(data + "\r\n");
}

void HttpResponse::makeResponse(HttpMethod::MethodType &method, int code, std::string &&data, bool keepAlive, Buffer &buf) {
    if (method == HttpMethod::OPTIONS) {
        // 浏览器跨域检查
        addResponseLine(204, "No Content", buf);
        addCors(buf);
    } else {
        addResponseLine(code, CODE[code], buf);
    }
    addHeaders(keepAlive, buf);
    if (!data.empty())
        addBody(std::forward<std::string>(data), buf);
    else {
        buf.append("Content-Length: 0\r\n\r\n");
    }
}


