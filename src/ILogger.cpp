#include "stormancer.h"

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

	void ILogger::log(wstring message)
	{
	}

	void ILogger::log(LogLevel level, wstring category, wstring message, wstring data)
	{
	}

	void ILogger::log(const std::exception& e)
	{
	}

	wstring ILogger::format(LogLevel level, wstring& category, wstring& message, wstring& data)
	{
		wstringstream ss;

		ss << L'[' << Helpers::nowStr() << L']';
		ss << L" [" << static_cast<int>(level) << L']';
		if (category.length())
		{
			ss << L" [" << category << L']';
		}
		if (message.length())
		{
			ss << L' ' << message;
		}
		if (data.length())
		{
			ss << L" [" << data << L']';
		}

		return ss.str();
	}

	wstring ILogger::formatException(const std::exception& e)
	{
		wstringstream ss;

		ss << L'[' << Helpers::nowStr() << L']';
		ss << L"[exception]";
		ss << L' ' << e.what();

		return ss.str();
	}
};
