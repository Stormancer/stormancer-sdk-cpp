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
	_mutex.lock();
	setConsoleColorWhite();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::log(Stormancer::LogLevel level, wstring category, wstring message, wstring data)
{
	if (level > _maximalLogLevel)
	{
		return;
	}

	wstring str = format(level, category, message, data);
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
		setConsoleColorDarkRed();
		break;
	case Stormancer::LogLevel::Fatal:
		setConsoleColorRed();
		break;
	default:
		setConsoleColorWhite();
		break;
	}
	wclog << str << endl;
	setConsoleColorWhite();
	_mutex.unlock();
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
	_mutex.unlock();
}

void ConsoleLogger::logGreen(wstring message)
{
	_mutex.lock();
	setConsoleColorGreen();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::logRed(wstring message)
{
	_mutex.lock();
	setConsoleColorRed();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::logBlue(wstring message)
{
	_mutex.lock();
	setConsoleColorBlue();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::logDarkRed(wstring message)
{
	_mutex.lock();
	setConsoleColorDarkRed();
	wclog << message << endl;
	_mutex.unlock();
}

void ConsoleLogger::logYellow(wstring message)
{
	_mutex.lock();
	setConsoleColorYellow();
	wclog << message << endl;
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
