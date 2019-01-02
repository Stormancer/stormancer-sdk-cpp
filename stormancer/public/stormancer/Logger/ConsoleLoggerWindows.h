#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Logger/ILogger.h"
#include <mutex>

#if defined(_WIN32)

#include <Windows.h>

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

		void setConsoleColor(WORD color);
		void resetColor();
		void setConsoleColorWhite();
		void setConsoleColorGrey();
		void setConsoleColorGreen();
		void setConsoleColorRed();
		void setConsoleColorBlue();
		void setConsoleColorYellow();
		void setConsoleColorDarkRed();

#pragma endregion

	protected:

#pragma region protected_members

		std::mutex _mutex;
		LogLevel _maximalLogLevel = LogLevel::Trace;
		bool _useBlueBackgroundColor = false;

#pragma endregion
	};
}

#endif
