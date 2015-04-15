#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	class DefaultLogger : public ILogger
	{
	public:
		static ILogger* instance();

		void log(LogLevel level, wstring category, wstring message, wstring data);

	protected:
		DefaultLogger();
		virtual ~DefaultLogger();
	};
};
