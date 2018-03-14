#include "stormancer/stdafx.h"
#include "stormancer/Logger/ConsoleLoggerWindows.h"

#if defined(_WIN32)

#include <iostream>

namespace Stormancer
{
	ConsoleLogger::ConsoleLogger(LogLevel maximalLogLevel)
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
		setConsoleColor(31);
	}

	void ConsoleLogger::setConsoleColorGrey()
	{
		setConsoleColor(24);
	}

	void ConsoleLogger::setConsoleColorGreen()
	{
		setConsoleColor(26);
	}

	void ConsoleLogger::setConsoleColorRed()
	{
		setConsoleColor(28);
	}

	void ConsoleLogger::setConsoleColorBlue()
	{
		setConsoleColor(27);
	}

	void ConsoleLogger::setConsoleColorDarkRed()
	{
		setConsoleColor(79);
	}

	void ConsoleLogger::setConsoleColorYellow()
	{
		setConsoleColor(30);
	}
}

#endif
