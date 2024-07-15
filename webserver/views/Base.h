//
// Created by wy on 24-7-6.
//

#ifndef TINYWEBSERVER_BASE_H
#define TINYWEBSERVER_BASE_H
#include "../http/HttpParams.h"
#include "../http/HttpResponse.h"
#include "../Type.h"
class Base {
public:
    virtual pis GET(HttpParams &params) { return {0, ""}; }
    virtual pis POST(HttpParams &params) { return {0, ""}; }
    virtual pis PUT(HttpParams &params) { return {0, ""}; }
    virtual pis DELETE(HttpParams &params) { return {0, ""}; }
    virtual pis OPTIONS(HttpParams &params) { return {0, ""}; }
    virtual bool isNull() {
        return true;
    }
};


#endif //TINYWEBSERVER_BASE_H
