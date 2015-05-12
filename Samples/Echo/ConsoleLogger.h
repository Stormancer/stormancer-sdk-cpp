#pragma once
#include <stormancer.h>

class ConsoleLogger : public Stormancer::ILogger
{
public:
	ConsoleLogger();
	virtual ~ConsoleLogger();

public:
	void log(Stormancer::LogLevel level, wstring category, wstring message, wstring data);
};
