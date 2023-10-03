#ifndef LOG_HPP
#define LOG_HPP

#include "utils/utils.hpp"

struct Logger
{

    const enum Level {
        INFO = 0,
        DBG,
        WARN,
        LOG_ERROR,
    };

    static void Log(std::string msg, Logger::Level logLevel=INFO, std::string source="Engine");
    static std::map<std::pair<int, std::string>, Logger::Level> loggedEntries;
};



#endif