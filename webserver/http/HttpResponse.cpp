//
// Created by wy on 24-7-3.
//

#include "HttpResponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
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
std::unordered_map<int, std::string> HttpResponse::CODE = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};
const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" },
};

void HttpResponse::init(const std::string& srcDir, const std::string& path, bool isKeepAlive, int code) {
    if (mmFile_) {
        unmapFile();
    }
    keepAlive_ = isKeepAlive;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = { 0 };
    code_ = code;
    path_ = path;
}

//void HttpResponse::addHeaders(bool keepAlive, Buffer &buf, int type) {
//    buf.append("Connection: ");
//    if (keepAlive) {
//        buf.append("keep-alive\r\n");
//        buf.append("keep-alive: max=6, timeout=120\r\n");
//    } else {
//        buf.append("close\r\n");
//    }
//    if (type) {
//        buf.append("Content-Type:application/json\r\n");
//    } else {
//        buf.append("Content-Type:text/html\r\n");
//    }
//    buf.append("Access-Control-Allow-Origin:*\r\n");
//}

void HttpResponse::addCors(Buffer &buf) {
    buf.append("Access-Control-Allow-Methods:POST, OPTIONS, GET, PUT, DELETE\r\n");
    buf.append("Access-Control-Allow-Headers:Content-Type, Connection, Content-Length, Keep-Alive, \r\n");
    buf.append("Access-Control-Max-Age:3600\r\n");
    buf.append("Cache-Control:no-cache, no-store, must-revalidate\r\n");
}

//void HttpResponse::addBody(const std::string &&data, Buffer &buf) {
//    buf.append("Content-Length: " + std::to_string(data.length()) + "\r\n\r\n");
//    buf.append(data + "\r\n");
//}

void HttpResponse::AddStateLine_(Buffer& buf) {
    std::string status;
    if(CODE.count(code_) == 1) {
        status = CODE.find(code_)->second;
    }
    else {
        code_ = 400;
        status = CODE.find(400)->second;
    }
    buf.append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer& buf) {
    buf.append("Connection: ");
    if(keepAlive_) {
        buf.append("keep-alive\r\n");
        buf.append("keep-alive: max=6, timeout=120\r\n");
    } else{
        buf.append("close\r\n");
    }
    buf.append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResponse::AddContent_(Buffer& buf) {
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if(srcFd < 0) {
        ErrorContent(buf, "File NotFound!");
        return;
    }

    /* 将文件映射到内存提高文件的访问速度
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s, size: %d", (srcDir_ + path_).data(), mmFileStat_.st_size);
    int* mmRet = (int*)mmap(nullptr, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        LOG_ERROR("map file failed");
        ErrorContent(buf, "File NotFound!");
        return;
    }
    mmFile_ = (char*)mmRet;
    close(srcFd);
    buf.append("Content-length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void HttpResponse::makeResponse(Buffer &buf) {
    /* 判断请求的资源文件 */
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) {
        code_ = 403;
    }
    else if(code_ == -1) {
        code_ = 200;
    }
    ErrorHtml_();
    AddStateLine_(buf);
    AddHeader_(buf);
    AddContent_(buf);
}

void HttpResponse::ErrorContent(Buffer& buff, std::string &&message)
{
    std::string body;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";
    buff.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}

void HttpResponse::unmapFile() {
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}
void HttpResponse::ErrorHtml_() {
    if(CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}
std::string HttpResponse::GetFileType_() {
    /* 判断文件类型 */
    std::string::size_type idx = path_.find_last_of('.');
    if(idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

char *HttpResponse::file() {
    return mmFile_;
}

size_t HttpResponse::fileLen() const {
    return mmFileStat_.st_size;
}


