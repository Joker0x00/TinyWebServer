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
    static std::string getResponse(int code, const std::string &&msg) {
        Json::CharReaderBuilder ReaderBuilder;
        ReaderBuilder["emitUTF8"] = true;//utf8支持,不加这句,utf8的中文字符会编程\uxxx
        std::unique_ptr<Json::CharReader> charRead(ReaderBuilder.newCharReader());
        Json::Value data;
        Json::Value json_temp;
        data["code"] = Json::Value(code);
        data["message"] = Json::Value(msg);
        data["data"] = Json::Value(json_temp);
        return jsonToStr(data);
    }
    static std::string jsonToStr(Json::Value &data) {
        Json::StreamWriterBuilder writeBuilder;
        writeBuilder["emitUTF8"] = true;//utf8支持,加这句,utf8的中文字符会编程\uxxx
        //2.把json对象转变为字符串
        return Json::writeString(writeBuilder, data);
    }

};

#endif //TINYWEBSERVER_RESPONSE_H
