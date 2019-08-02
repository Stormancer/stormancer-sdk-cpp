#include "stormancer/stdafx.h"
#include "stormancer/Logger/NullLogger.h"
#include "stormancer/Helpers.h"
#include <sstream>
#include <exception>

namespace Stormancer
{

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

	void ILogger::log(LogLevel level, const std::string& category, const std::string& message, const std::exception& exception)
	{
		std::string formattedException = "Exception:\n\t";
		formatNestedExceptionRecursive(exception, formattedException);
		log(level, category, message, formattedException);
	}

	void ILogger::formatNestedExceptionRecursive(const std::exception& ex, std::string& output)
	{
		output += std::string("[") + ex.what() + "]";
		try
		{
			std::rethrow_if_nested(ex);
		}
		catch (const std::exception& ex)
		{
			// One more nested exception, increase indentation
			output += "\n\t";
			formatNestedExceptionRecursive(ex, output);
		}
		catch (...) {}
	}
}
