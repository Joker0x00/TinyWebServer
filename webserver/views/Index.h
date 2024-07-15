//
// Created by wy on 24-7-15.
//

#ifndef TINYWEBSERVER_INDEX_H
#define TINYWEBSERVER_INDEX_H
#include "Base.h"
class Index: public Base{
public:
    pis GET(HttpParams &params) override {
        return {1, "index.html"};
    }
    bool isNull() override {
        return false;
    }
};

#endif //TINYWEBSERVER_INDEX_H
