#pragma once

#include "stormancer/headers.h"
#include "stormancer/Logger/ILogger.h"

#if defined(_WIN32)

namespace Stormancer
{
	class ConsoleLogger : public ILogger
	{
	public:

#pragma region public_methods

		ConsoleLogger(LogLevel maximalLogLevel = LogLevel::Trace);
		~ConsoleLogger();
		void log(const std::string& message) override;
		void log(LogLevel level, const std::string& category, const std::string& message, const std::string& data = "") override;
		void log(const std::exception& ex) override;

		static void setConsoleColor(WORD color);
		static void setConsoleColorWhite();
		static void setConsoleColorGrey();
		static void setConsoleColorGreen();
		static void setConsoleColorRed();
		static void setConsoleColorBlue();
		static void setConsoleColorDarkRed();
		static void setConsoleColorYellow();

#pragma endregion

	protected:

#pragma region protected_members

		std::mutex _mutex;
		LogLevel _maximalLogLevel = LogLevel::Trace;

#pragma endregion
	};
}

#endif
