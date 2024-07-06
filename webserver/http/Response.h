//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_RESPONSE_H
#define TINYWEBSERVER_RESPONSE_H

#include <string>
#include <memory>
#include <utility>

#include "../lib/json/json.h"
class HttpReturnStatus {

};

class Response {
public:
    int code_;
    const std::string message_;
    std::shared_ptr<Json::Value> data_;

    Response(int code, const std::string message, const std::shared_ptr<Json::Value> data) : code_(code),
                                                                                               message_(message),
                                                                                               data_(data) {
    }
    Response(int code) {

    }
    std::string jsonToStr() {
        Json::StreamWriterBuilder writeBuilder;
        writeBuilder["emitUTF8"] = true;//utf8支持,加这句,utf8的中文字符会编程\uxxx
        //2.把json对象转变为字符串
        return Json::writeString(writeBuilder, *data_);
    }
    bool hasData() {
        return data_ != nullptr;
    }
};

#endif //TINYWEBSERVER_RESPONSE_H
