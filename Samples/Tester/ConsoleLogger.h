#pragma once
#include "stormancer.h"

class ConsoleLogger : public Stormancer::ILogger
{
public:
	ConsoleLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
	virtual ~ConsoleLogger();

public:
	void log(const std::string message);
	void log(Stormancer::LogLevel level, const std::string category, const std::string message, const std::string data);
	void log(const std::exception& e);

public:
	static void ConsoleLogger::setConsoleColor(WORD color);
	static void ConsoleLogger::setConsoleColorWhite();
	static void ConsoleLogger::setConsoleColorGrey();
	static void ConsoleLogger::setConsoleColorGreen();
	static void ConsoleLogger::setConsoleColorRed();
	static void ConsoleLogger::setConsoleColorBlue();
	static void ConsoleLogger::setConsoleColorDarkRed();
	static void ConsoleLogger::setConsoleColorYellow();

private:
	std::mutex _mutex;
	Stormancer::LogLevel _maximalLogLevel = Stormancer::LogLevel::Trace;
};
