#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	class NullLogger : public ILogger
	{
	public:
		NullLogger();
		virtual ~NullLogger();

	public:
		void log(LogLevel level, wstring category, wstring message, wstring data);
	};
};
