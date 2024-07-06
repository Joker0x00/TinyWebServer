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


// 构造HTTP响应报文
class HttpResponse {
private:
    std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    std::unordered_map<int, std::string> CODE;
    std::unordered_map<int, std::string>  CODE_TO_HTML;
    int code_;
    std::string title_;
    bool keepAlive_;

    char* mmFile_;
    struct stat mmFileStat_{};

    std::string path_;
    std::string srcDir_;
public:
    HttpResponse();
    ~HttpResponse();
    void init(std::string &path, std::string &srcDir, bool keepAlive, int code);

    std::string getFileType();
    size_t getFileLen() const;
    char* getFile();
    void unmapFile();
    void addResponseLine(Buffer &buf);
    void addHeaders(Buffer &buf);
    void addBody(Buffer &buf);
    void ErrorContent(Buffer& buff, const std::string& message);
    void ErrorHtml();

    void makeResponse(Buffer &buf);

    void makeResponse(std::shared_ptr<Response> res, bool keepAlive, Buffer &buf);

    void addResponseLine(int code, const std::string &title, Buffer &buf);

    void addHeaders(bool keepAlive, Buffer &buf);

    void addBody(const std::string &data, Buffer &buf);


};


#endif //TINYWEBSERVER_HTTPRESPONSE_H
