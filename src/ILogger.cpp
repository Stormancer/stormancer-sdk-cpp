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

	RakNet::RakString ILogger::format(LogLevel level, const char* category, const char* message, const char* data)
	{
		std::stringstream ss;

		ss << '[' << nowStr() << ']';
		ss << " [" << static_cast<int>(level) << ']';
		if (std::strlen(category))
		{
			ss << " [" << category << ']';
		}
		if (std::strlen(message))
		{
			ss << ' ' << message;
		}
		if (std::strlen(data))
		{
			ss << " [" << data << ']';
		}

		return RakNet::RakString(ss.str().c_str());
	}

	RakNet::RakString ILogger::formatException(const std::exception& e)
	{
		std::stringstream ss;
		ss << '[' << nowStr() << ']' << " [exception] " << e.what() << std::endl;
		return RakNet::RakString(ss.str().c_str());
	}
};
