#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <map>

#ifndef AURORA_DLLBUILD
#define AURORA_API __declspec(dllexport)
#else
#define AURORA_API __declspec(dllimport)
#endif


struct Logger
{

	enum Level {
        INFO = 0,
        DBG,
        WARN,
        LOG_ERROR,
    };

	AURORA_API static void Log(std::string msg, Logger::Level logLevel=INFO, std::string source="Engine");
	inline static std::map<std::pair<int, std::string>, Logger::Level> loggedEntries = std::map<std::pair<int, std::string>, Logger::Level>();
};



#endif