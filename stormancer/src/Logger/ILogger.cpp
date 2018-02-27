#include "stormancer/stdafx.h"
#include "stormancer/Logger/NullLogger.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	std::shared_ptr<ILogger> ILogger::sLogger;

	std::shared_ptr<ILogger> ILogger::instance()
	{
		return sLogger;
	}

	void ILogger::setInstance(std::shared_ptr<ILogger> logger)
	{
		sLogger = logger;
	}

	ILogger::ILogger()
	{
	}

	ILogger::~ILogger()
	{
	}

	std::string ILogger::format(LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		std::stringstream ss;

		ss << '[' << nowStr() << ']';

		switch (level)
		{
			case LogLevel::Fatal:
				ss << " [Fatal]";
				break;
			case LogLevel::Error:
				ss << " [Error]";
				break;
			case LogLevel::Warn:
				ss << " [Warn ]";
				break;
			case LogLevel::Info:
				ss << " [Info ]";
				break;
			case LogLevel::Debug:
				ss << " [Debug]";
				break;
			case LogLevel::Trace:
				ss << " [Trace]";
				break;
		}

		if (!category.empty())
		{
			ss << " [" << category << ']';
		}

		if (!message.empty())
		{
			ss << ' ' << message;
		}

		if (!data.empty())
		{
			ss << " [" << data << ']';
		}

		return ss.str();
		
	}

	std::string ILogger::formatException(const std::exception& ex)
	{
		return format(LogLevel::Error, "exception", ex.what());
	}
};
