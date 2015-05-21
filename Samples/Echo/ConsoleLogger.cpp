#include "ConsoleLogger.h"

ConsoleLogger::ConsoleLogger(Stormancer::LogLevel maximalLogLevel)
	: _maximalLogLevel(maximalLogLevel)
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::log(wstring message)
{
	logWhite(message);
}

void ConsoleLogger::log(Stormancer::LogLevel level, wstring category, wstring message, wstring data)
{
	if (level > _maximalLogLevel)
	{
		return;
	}

	wstring message2 = format(level, category, message, data);
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
	wclog << formatException(e);
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logWhite(wstring message)
{
	_mutex.lock();
	setConsoleColorWhite();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::logGrey(wstring message)
{
	_mutex.lock();
	setConsoleColorGrey();
	wclog << message << endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logGreen(wstring message)
{
	_mutex.lock();
	setConsoleColorGreen();
	wclog << message << endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logRed(wstring message)
{
	_mutex.lock();
	setConsoleColorRed();
	wclog << message << endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logBlue(wstring message)
{
	_mutex.lock();
	setConsoleColorBlue();
	wclog << message << endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logDarkRed(wstring message)
{
	_mutex.lock();
	setConsoleColorDarkRed();
	wclog << message << endl;
	setConsoleColorWhite();
	_mutex.unlock();
}

void ConsoleLogger::logYellow(wstring message)
{
	_mutex.lock();
	setConsoleColorYellow();
	wclog << message << endl;
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
