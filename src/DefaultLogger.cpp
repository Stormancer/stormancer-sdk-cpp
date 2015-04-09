#include "stormancer.h"

namespace Stormancer
{
	ILogger* DefaultLogger::instance()
	{
		static ILogger* defaultLogger = new DefaultLogger;
		return defaultLogger;
	}

	DefaultLogger::DefaultLogger()
	{
	}

	DefaultLogger::~DefaultLogger()
	{
	}

	void DefaultLogger::log(LogLevel level, wstring category, wstring message, wstring data)
	{
		wcout << Helpers::nowStr() << endl;
		wcout << L"level: " << static_cast<int>(level);
		if (category.length())
		{
			wcout << L"category: " << category << endl;
		}
		if (message.length())
		{
			wcout << L"message: " << message << endl;
		}
		if (data.length())
		{
			wcout << L"data: " << data << endl;
		}
		wcout << endl;
	}
};
