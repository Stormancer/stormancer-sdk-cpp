#include "stormancer/stdafx.h"
#include "stormancer/Logger/ConsoleLoggerWindows.h"

#if defined(_WIN32)

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
				setConsoleColorWhite();
				break;
		}

		std::clog << message2 << std::endl;

		setConsoleColorWhite();
	}

	void ConsoleLogger::log(const std::exception& e)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		setConsoleColorRed();

		std::clog << formatException(e) << std::endl;

		setConsoleColorWhite();
	}

	void ConsoleLogger::setConsoleColor(WORD color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	void ConsoleLogger::setConsoleColorWhite()
	{
		setConsoleColor(15);
	}

	void ConsoleLogger::setConsoleColorGrey()
	{
		setConsoleColor(8);
	}

	void ConsoleLogger::setConsoleColorGreen()
	{
		setConsoleColor(2);
	}

	void ConsoleLogger::setConsoleColorRed()
	{
		setConsoleColor(12);
	}

	void ConsoleLogger::setConsoleColorBlue()
	{
		setConsoleColor(11);
	}

	void ConsoleLogger::setConsoleColorDarkRed()
	{
		setConsoleColor(4);
	}

	void ConsoleLogger::setConsoleColorYellow()
	{
		setConsoleColor(14);
	}
}

#endif
