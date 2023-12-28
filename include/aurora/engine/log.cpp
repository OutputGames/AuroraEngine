#include "log.hpp"

#include <iostream>

void Logger::Log(std::string msg, Logger::Level logLevel, std::string source)
{
	loggedEntries.insert({ {loggedEntries.size(),"[" + source + "] " + msg}, logLevel });

	std::cout << msg << std::endl;
}