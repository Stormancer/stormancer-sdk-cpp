#include "stormancer.h"
#include "NullLogger.h"

namespace Stormancer
{
	ILogger* ILogger::_logger = nullptr;

	ILogger* ILogger::instance()
	{
		if (!ILogger::_logger)
		{
			ILogger::_logger = new NullLogger;
		}
		return ILogger::_logger;
	}

	ILogger* ILogger::instance(ILogger* logger)
	{
		_logger = logger;
		return _logger;
	}

	ILogger::ILogger()
	{
	}

	ILogger::~ILogger()
	{
	}

	std::string ILogger::format(LogLevel level, std::string& category, std::string& message, std::string& data)
	{
		std::stringstream ss;

		ss << '[' << nowStr() << ']';
		ss << " [" << static_cast<int>(level) << ']';
		if (category.length())
		{
			ss << " [" << category << ']';
		}
		if (message.length())
		{
			ss << ' ' << message;
		}
		if (data.length())
		{
			ss << " [" << data << ']';
		}

		return ss.str();
	}

	std::string ILogger::formatException(const std::exception& e)
	{
		std::stringstream ss;
		ss << '[' << nowStr() << ']' << " [exception] " << e.what() << std::endl;
		return ss.str();
	}
};
