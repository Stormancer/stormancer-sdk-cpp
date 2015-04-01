#include "stormancer.h"

namespace Stormancer
{
	shared_ptr<ILogger> DefaultLogger::instance()
	{
		static shared_ptr<ILogger> defaultLogger = make_shared<ILogger>(new DefaultLogger);
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
		time_t t = time(nullptr);
		wcout << put_time(localtime(&t), "%F %T") << endl;
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
