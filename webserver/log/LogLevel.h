//
// Created by wy on 24-7-2.
//

#ifndef TINYWEBSERVER_LOGLEVEL_H
#define TINYWEBSERVER_LOGLEVEL_H
class LogLevel
{
public:

    enum class value
    {
        UNKNOW =0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    static const char *toString(value level)
    {
        switch (level)
        {
            case LogLevel::value::DEBUG: return "[DEBUG]:";
            case LogLevel::value::INFO: return  "[INFO] :";
            case LogLevel::value::WARN: return  "[WARN] :";
            case LogLevel::value::ERROR: return "[ERROR]:";
            case LogLevel::value::FATAL: return "[FATAL]:";
            case LogLevel::value::OFF: return   "[OFF]  :";
            default: return "UNKNOW";
        }
    }
};
#endif //TINYWEBSERVER_LOGLEVEL_H
