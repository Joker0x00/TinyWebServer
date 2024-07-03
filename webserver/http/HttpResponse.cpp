//
// Created by wy on 24-7-3.
//

#include "HttpResponse.h"

HttpResponse::HttpResponse() {
    code_ = -1;
    title_ = "";
    keepAlive_ = false;
    mmFileStat_ = {0};
    mmFile_ = nullptr;
    SUFFIX_TYPE = {
            { ".html",  "text/html" },
            { ".xml",   "text/xml" },
            { ".xhtml", "application/xhtml+xml" },
            { ".txt",   "text/plain" },
            { ".rtf",   "application/rtf" },
            { ".pdf",   "application/pdf" },
            { ".word",  "application/nsword" },
            { ".png",   "image/png" },
            { ".gif",   "image/gif" },
            { ".jpg",   "image/jpeg" },
            { ".jpeg",  "image/jpeg" },
            { ".au",    "audio/basic" },
            { ".mpeg",  "video/mpeg" },
            { ".mpg",   "video/mpeg" },
            { ".avi",   "video/x-msvideo" },
            { ".gz",    "application/x-gzip" },
            { ".tar",   "application/x-tar" },
            { ".css",   "text/css "},
            { ".js",    "text/javascript "},
    };
    CODE = {
            { 200, "OK" },
            { 400, "Bad Request" },
            { 403, "Forbidden" },
            { 404, "Not Found" },
    };
    CODE_TO_HTML = {
            { 400, "/400.html" },
            { 403, "/403.html" },
            { 404, "/404.html" },
    };
}

HttpResponse::~HttpResponse() {
    unmapFile();
}

void HttpResponse::init(std::string &path, std::string &srcDir, bool keepAlive, int code) {
    path_ = path;
    srcDir_ = srcDir;
    keepAlive_ = keepAlive;
    code_ = code;
    mmFileStat_ = {0};
    mmFile_ = nullptr;
}

void HttpResponse::unmapFile() {
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

void HttpResponse::addResponseLine(Buffer &buf) {
    std::string res;
    util::String::formatPrintStr(res, "%s %d %s\r\n", "HTTP/1.1", code_, title_.c_str());
    buf.append(res);
}

void HttpResponse::addHeaders(Buffer &buf) {
    buf.append("Connection: ");
    if (keepAlive_) {
        buf.append("keep-alive\r\n");
        buf.append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buf.append("close\r\n");
    }
    buf.append("Content-type: " + getFileType() + "\r\n");
}

std::string HttpResponse::getFileType() {
    auto idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if (!SUFFIX_TYPE.count(suffix)) {
        return "text/plain";
    }
    return SUFFIX_TYPE[suffix];
}

void HttpResponse::addBody(Buffer &buf) {
    buf.append("Content-Length: " + std::to_string(getFileLen()) + "\r\n\r\n");
    int srcFd = open((srcDir_ + path_).c_str(), O_RDONLY);
    if (srcFd < 0) {
        ErrorContent(buf, "File Not Found");
        return ;
    }
    // 将文件映射到内存中
    int *addr = (int*)(mmap(nullptr, getFileLen(), PROT_READ, MAP_PRIVATE, srcFd,
                                       0));
    if (*addr == -1) {
        ErrorContent(buf, "File NotFound!");
        return ;
    }
    mmFile_ = (char*)(addr);
    close(srcFd);
}

size_t HttpResponse::getFileLen() const {
    return mmFileStat_.st_size;
}
void HttpResponse::ErrorContent(Buffer& buff, const std::string& message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE.count(code_) == 1) {
        status = CODE.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}

void HttpResponse::ErrorHtml() {
    if(CODE.count(code_) == 1) {
        path_ = CODE.find(code_)->second;
        // 获取文件的状态信息
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

void HttpResponse::makeResponse(Buffer &buf) {
    if (code_ == 200) {
        if (stat((srcDir_ + path_).c_str(), &mmFileStat_) < 0  || S_ISDIR(mmFileStat_.st_mode)) {
            code_ = 404;
        } else if (!(mmFileStat_.st_mode & S_IROTH)) { // 是否拥有读权限
            code_ = 403;
        }
    }
    ErrorHtml();
    addResponseLine(buf);
    addHeaders(buf);
    addBody(buf);
}

char * HttpResponse::getFile() {
    return mmFile_;
}



