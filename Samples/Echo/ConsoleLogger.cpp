#include "ConsoleLogger.h"

ConsoleLogger::ConsoleLogger(Stormancer::LogLevel maximalLogLevel)
	: _maximalLogLevel(maximalLogLevel)
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::log(std::string message)
{
	logWhite(message);
}

void ConsoleLogger::log(Stormancer::LogLevel level, std::string category, std::string message, std::string data)
{
	if (level > _maximalLogLevel)
	{
		return;
	}

	std::string message2 = format(level, category, message, data);
	switch (level)
	{
	case Stormancer::LogLevel::Trace:
		logGrey(message2);
		break;
	case Stormancer::LogLevel::Debug:
		logWhite(message2);
		break;
	case Stormancer::LogLevel::Info:
		logBlue(message2);
		break;
	case Stormancer::LogLevel::Warn:
		logYellow(message2);
		break;
	case Stormancer::LogLevel::Error:
		logDarkRed(message2);
		break;
	case Stormancer::LogLevel::Fatal:
		logRed(message2);
		break;
	default:
		logWhite(message2);
		break;
	}
}

void ConsoleLogger::log(const std::exception& e)
{
	_mutex.lock();
	setConsoleColorRed();
	std::clog << formatException(e);
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logWhite(std::string message)
{
	_mutex.lock();
	setConsoleColorWhite();
	std::clog << message << std::endl;
	_mutex.unlock();
}

void ConsoleLogger::logGrey(std::string message)
{
	_mutex.lock();
	setConsoleColorGrey();
	std::clog << message << std::endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logGreen(std::string message)
{
	_mutex.lock();
	setConsoleColorGreen();
	std::clog << message << std::endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logRed(std::string message)
{
	_mutex.lock();
	setConsoleColorRed();
	std::clog << message << std::endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logBlue(std::string message)
{
	_mutex.lock();
	setConsoleColorBlue();
	std::clog << message << std::endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logDarkRed(std::string message)
{
	_mutex.lock();
	setConsoleColorDarkRed();
	std::clog << message << std::endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logYellow(std::string message)
{
	_mutex.lock();
	setConsoleColorYellow();
	std::clog << message << std::endl;
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
