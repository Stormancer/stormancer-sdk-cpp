#include "stormancer/stdafx.h"
#include "stormancer/Logger/ConsoleLogger.h"

#if defined(_WIN32)
#else

namespace Stormancer
{
	ConsoleLogger::ConsoleLogger(Stormancer::LogLevel maximalLogLevel)
		: _maximalLogLevel(maximalLogLevel)
	{
	}

	ConsoleLogger::~ConsoleLogger()
	{
	}

	void ConsoleLogger::log(const std::string& message)
	{
		log(Stormancer::LogLevel::Debug, "None", message);
	}

	void ConsoleLogger::log(Stormancer::LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		if (level > _maximalLogLevel)
		{
			return;
		}

		std::string message2 = format(level, category, message, data);

		std::lock_guard<std::mutex> lg(_mutex);

		std::clog << message2 << std::endl;
	}

	void ConsoleLogger::log(const std::exception& e)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		std::clog << formatException(e) << std::endl;
	}
}

#endif
