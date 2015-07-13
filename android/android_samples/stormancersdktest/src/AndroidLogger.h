#pragma once
#include <stormancer.h>

class AndroidLogger : public Stormancer::ILogger
{
public:
	AndroidLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
	virtual ~AndroidLogger();

public:
	void log(std::string message);
	void log(Stormancer::LogLevel level, std::string category, std::string message, std::string data);
	void log(const std::exception& e);

public:
	Stormancer::LogLevel _maximalLogLevel = Stormancer::LogLevel::Trace;
};
