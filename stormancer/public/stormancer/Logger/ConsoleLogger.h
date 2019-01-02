#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Logger/ILogger.h"
#include <mutex>

#if defined(_WIN32)
#include "stormancer/Logger/ConsoleLoggerWindows.h"
#else

namespace Stormancer
{
	class ConsoleLogger : public ILogger
	{
	public:

#pragma region public_methods

		ConsoleLogger(LogLevel maximalLogLevel = LogLevel::Trace);
		virtual ~ConsoleLogger();
		void log(const std::string& message) override;
		void log(LogLevel level, const std::string& category, const std::string& message, const std::string& data = "") override;
		void log(const std::exception& e) override;

#pragma endregion

	protected:

#pragma region protected_members

		std::mutex _mutex;
		LogLevel _maximalLogLevel = LogLevel::Trace;

#pragma endregion
	};
}

#endif
