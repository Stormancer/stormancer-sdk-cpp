#include "stormancer.h"
#include "NullLogger.h"

namespace Stormancer
{
	std::shared_ptr<ILogger> ILogger::_logger;

	std::shared_ptr<ILogger> ILogger::instance()
	{
		if (!ILogger::_logger)
		{
			ILogger::_logger = std::make_shared<NullLogger>();
		}
		return ILogger::_logger;
	}

	std::shared_ptr<ILogger> ILogger::instance(std::shared_ptr<ILogger> logger)
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

	std::shared_ptr<const char> ILogger::format(LogLevel level, const char* category, const char* message, const char* data)
	{
		std::stringstream ss;

		ss << '[' << nowStr() << ']';
		ss << " [" << static_cast<int>(level) << ']';
		if (category && std::strlen(category))
		{
			ss << " [" << category << ']';
		}
		if (message && std::strlen(message))
		{
			ss << ' ' << message;
		}
		if (data && std::strlen(data))
		{
			ss << " [" << data << ']';
		}

		auto str = ss.str();
		auto dataToCopy = str.c_str();
		auto len = strlen(dataToCopy) + 1;
		auto dataToReturn = new char[len];
		memcpy(dataToReturn, dataToCopy, len);
		return std::shared_ptr<const char>(dataToReturn, array_deleter<const char>());
	}

	std::shared_ptr<const char> ILogger::formatException(const std::exception& e)
	{
		std::stringstream ss;
		ss << '[' << nowStr() << ']' << " [exception] " << e.what() << std::endl;
		auto str = ss.str();
		auto dataToCopy = str.c_str();
		auto len = strlen(dataToCopy) + 1;
		auto data = new char[len];
		memcpy(data, dataToCopy, len);
		return std::shared_ptr<const char>(data, array_deleter<const char>());
	}
};
