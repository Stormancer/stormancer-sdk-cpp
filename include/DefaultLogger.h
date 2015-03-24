#pragma once
#include "headers.h"
#include "Core/ILogger.h"

namespace Stormancer
{
	class DefaultLogger : public ILogger
	{
	public:
		static shared_ptr<ILogger*> instance();

		void Log(LogLevel level, string category, string message, string data);

	protected:
		DefaultLogger();
		virtual ~DefaultLogger();
	};
};
