#include "stormancer/stdafx.h"
#include "stormancer/Logger/ConsoleLoggerWindows.h"

#if defined(_WIN32)

#include <iostream>

namespace Stormancer
{
	ConsoleLogger::ConsoleLogger(LogLevel maximalLogLevel)
		: _maximalLogLevel(maximalLogLevel)
		, _useBlueBackgroundColor(false)
	{
	}

	ConsoleLogger::~ConsoleLogger()
	{
	}

	void ConsoleLogger::log(const std::string& message)
	{
		log(Stormancer::LogLevel::Debug, "", message);
	}

	void ConsoleLogger::log(LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		if (level > _maximalLogLevel)
		{
			return;
		}

		std::string message2 = format(level, category, message, data);

		std::lock_guard<std::mutex> lg(_mutex);

		switch (level)
		{
			case LogLevel::Trace:
				setConsoleColorGrey();
				break;
			case LogLevel::Debug:
				setConsoleColorWhite();
				break;
			case LogLevel::Info:
				setConsoleColorBlue();
				break;
			case LogLevel::Warn:
				setConsoleColorYellow();
				break;
			case LogLevel::Error:
				setConsoleColorRed();
				break;
			case LogLevel::Fatal:
				setConsoleColorDarkRed();
				break;
			default:
				resetColor();
				break;
		}

		std::clog << message2 << std::endl;

		resetColor();
	}

	void ConsoleLogger::log(const std::exception& ex)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		setConsoleColorRed();

		std::clog << formatException(ex) << std::endl;

		resetColor();
	}

	void ConsoleLogger::setConsoleColor(WORD color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	void ConsoleLogger::resetColor()
	{
		setConsoleColor(15);
	}

	void ConsoleLogger::setConsoleColorWhite()
	{
		setConsoleColor(_useBlueBackgroundColor ? 31 : 15);
	}

	void ConsoleLogger::setConsoleColorGrey()
	{
		setConsoleColor(_useBlueBackgroundColor ? 24 : 8);
	}

	void ConsoleLogger::setConsoleColorGreen()
	{
		setConsoleColor(_useBlueBackgroundColor ? 26 : 10);
	}

	void ConsoleLogger::setConsoleColorRed()
	{
		setConsoleColor(_useBlueBackgroundColor ? 28 : 12);
	}

	void ConsoleLogger::setConsoleColorBlue()
	{
		setConsoleColor(_useBlueBackgroundColor ? 27 : 11);
	}

	void ConsoleLogger::setConsoleColorYellow()
	{
		setConsoleColor(_useBlueBackgroundColor ? 30 : 14);
	}

	void ConsoleLogger::setConsoleColorDarkRed()
	{
		setConsoleColor(79);
	}
}

#endif
