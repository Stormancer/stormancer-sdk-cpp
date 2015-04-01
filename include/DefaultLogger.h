#pragma once
#include "headers.h"
#include "Core/ILogger.h"

namespace Stormancer
{
	class DefaultLogger : public ILogger
	{
	public:
		static shared_ptr<ILogger> instance();

		void log(LogLevel level, wstring category, wstring message, wstring data);

	protected:
		DefaultLogger();
		virtual ~DefaultLogger();
	};
};
