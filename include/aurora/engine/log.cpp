#include "log.hpp"

std::map<std::pair<int, std::string>, Logger::Level> Logger::loggedEntries;

void Logger::Log(std::string msg, Logger::Level logLevel, std::string source)
{
	loggedEntries.insert({ {loggedEntries.size(),"[" + source + "] " + msg}, logLevel });

	cout << msg << std::endl;
}