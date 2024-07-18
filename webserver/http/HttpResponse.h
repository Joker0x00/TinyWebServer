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

// 构造HTTP响应报文
class HttpResponse {
private:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_PATH;
    static std::unordered_map<int, std::string> CODE;
    std::string srcDir_;
    std::string path_;
    bool keepAlive_{};
    int code_{};
    char* mmFile_{};
    struct stat mmFileStat_{};
public:

    HttpResponse() = default;
    ~HttpResponse() = default;

    static void addCors(Buffer &buf);

    static void ErrorContent(Buffer &buff, std::string &&message);

    void unmapFile();

    void makeResponse(Buffer &buf);

    void init(const std::string &srcDir, const std::string &path, bool isKeepAlive, int code);

    void ErrorHtml_();

    void AddStateLine_(Buffer &buf);

    void AddHeader_(Buffer &buf);

    void AddContent_(Buffer &buff);

    std::string GetFileType_();

    size_t fileLen() const;

    char* file();
};


#endif //TINYWEBSERVER_HTTPRESPONSE_H
