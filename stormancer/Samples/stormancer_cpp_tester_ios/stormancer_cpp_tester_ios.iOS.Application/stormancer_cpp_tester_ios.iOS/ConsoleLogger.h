#pragma once
#include "stormancer.h"

typedef int16_t WORD;

class ConsoleLogger : public Stormancer::ILogger
{
public:
	ConsoleLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
	virtual ~ConsoleLogger();

public:
	virtual void log(const std::string message) override;
	virtual void log(Stormancer::LogLevel level, const std::string category, const std::string message, const std::string data = "") override;
	virtual void log(const std::exception& e) override;

public:
	static void setConsoleColor(WORD color);
	static void setConsoleColorWhite();
	static void setConsoleColorGrey();
	static void setConsoleColorGreen();
	static void setConsoleColorRed();
	static void setConsoleColorBlue();
	static void setConsoleColorDarkRed();
	static void setConsoleColorYellow();

private:
	std::mutex _mutex;
	Stormancer::LogLevel _maximalLogLevel = Stormancer::LogLevel::Trace;
};
