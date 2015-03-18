#pragma once
#include "stdafx.h"
#include "Core/ILogger.h"

namespace Stormancer
{
	class DefaultLogger : public ILogger
	{
	public:
		static DefaultLogger& instance();

		void Log(LogLevel level, string category, string message, string data);

	protected:
		DefaultLogger();
		virtual ~DefaultLogger();
	};
};
