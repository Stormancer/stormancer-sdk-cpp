#pragma once
#include <stormancer.h>

class ConsoleLogger : public Stormancer::ILogger
{
public:
	ConsoleLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
	virtual ~ConsoleLogger();

public:
	void log(wstring message);
	void log(Stormancer::LogLevel level, wstring category, wstring message, wstring data);
	void log(const std::exception& e);

	void logWhite(wstring message);
	void logGrey(wstring message);
	void logGreen(wstring message);
	void logRed(wstring message);
	void logBlue(wstring message);
	void logDarkRed(wstring message);
	void logYellow(wstring message);

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
