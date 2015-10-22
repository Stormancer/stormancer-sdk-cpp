#pragma once
#include <stormancer.h>

class ConsoleLogger : public Stormancer::ILogger
{
public:
	ConsoleLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
	virtual ~ConsoleLogger();

public:
	void log(std::string message);
	void log(Stormancer::LogLevel level, std::string category, std::string message, std::string data);
	void log(const std::exception& e);

	void logWhite(std::string message);
	void logGrey(std::string message);
	void logGreen(std::string message);
	void logRed(std::string message);
	void logBlue(std::string message);
	void logDarkRed(std::string message);
	void logYellow(std::string message);

	static void ConsoleLogger::setConsoleColor(WORD color);
	static void ConsoleLogger::setConsoleColorWhite();
	static void ConsoleLogger::setConsoleColorGrey();
	static void ConsoleLogger::setConsoleColorGreen();
	static void ConsoleLogger::setConsoleColorRed();
	static void ConsoleLogger::setConsoleColorBlue();
	static void ConsoleLogger::setConsoleColorDarkRed();
	static void ConsoleLogger::setConsoleColorYellow();

public:
	Stormancer::LogLevel _maximalLogLevel = Stormancer::LogLevel::Trace;
};
