
#include "DefaultLogger.h"

namespace Stormancer
{
	shared_ptr<ILogger*> DefaultLogger::instance()
	{
		static shared_ptr<ILogger*> defaultLogger = make_shared<ILogger*>(new DefaultLogger());
		return defaultLogger;
	}

	DefaultLogger::DefaultLogger()
	{
	}

	DefaultLogger::~DefaultLogger()
	{
	}

	void DefaultLogger::Log(LogLevel level, string category, string message, string data)
	{
		time_t t = time(nullptr);
		cout << put_time(localtime(&t), "%F %T") << endl;
		cout << "level: " << static_cast<int>(level);
		if (category.length())
		{
			cout << "category: " << category << endl;
		}
		if (message.length())
		{
			cout << "message: " << message << endl;
		}
		if (data.length())
		{
			cout << "data: " << data << endl;
		}
		cout << endl;
	}
};
