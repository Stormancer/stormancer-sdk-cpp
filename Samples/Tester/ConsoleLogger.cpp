#include "ConsoleLogger.h"

ConsoleLogger::ConsoleLogger(Stormancer::LogLevel maximalLogLevel)
	: _maximalLogLevel(maximalLogLevel)
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::log(const char* message)
{
	log(Stormancer::LogLevel::Info, "None", message, "");
}

void ConsoleLogger::log(Stormancer::LogLevel level, const char* category, const char* message, const char* data)
{
	if (level > _maximalLogLevel)
	{
		return;
	}
	RakNet::RakString message2 = format(level, category, message, data);

	_mutex.lock();

	switch (level)
	{
	case Stormancer::LogLevel::Trace:
		setConsoleColorGrey();
		break;
	case Stormancer::LogLevel::Debug:
		setConsoleColorWhite();
		break;
	case Stormancer::LogLevel::Info:
		setConsoleColorBlue();
		break;
	case Stormancer::LogLevel::Warn:
		setConsoleColorYellow();
		break;
	case Stormancer::LogLevel::Error:
		setConsoleColorRed();
		break;
	case Stormancer::LogLevel::Fatal:
		setConsoleColorDarkRed();
		break;
	default:
		setConsoleColorWhite();
		break;
	}

	std::clog << message2.C_String() << std::endl;

	setConsoleColorWhite();

	_mutex.unlock();
}

void ConsoleLogger::log(const std::exception& e)
{
	_mutex.lock();
	setConsoleColorRed();
	std::clog << formatException(e);
	setConsoleColorWhite();
	_mutex.unlock();
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
