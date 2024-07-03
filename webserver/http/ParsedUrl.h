//
// Created by wy on 24-7-3.
//

#ifndef TINYWEBSERVER_PARSEDURL_H
#define TINYWEBSERVER_PARSEDURL_H
#include <string>
#include <unordered_map>

class ParsedUrl {
public:
    std::string path;
    std::unordered_map<std::string, std::string> queryParams;
};


#endif //TINYWEBSERVER_PARSEDURL_H
