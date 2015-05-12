#include "ConsoleLogger.h"

ConsoleLogger::ConsoleLogger()
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::log(Stormancer::LogLevel level, wstring category, wstring message, wstring data)
{
	wstring str = format(level, category, message, data);
	wclog << str;
	wclog.flush();
}
