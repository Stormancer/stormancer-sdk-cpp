#pragma once
#include "headers.h"

namespace Stormancer
{
	// The available log levels.
	enum class LogLevel
	{
		// Applies to critical errors that prevent the program from continuing.
		Fatal,
		// Applies to non critical errors
		Error,
		// Applies to warnings
		Warn,
		// Applies to information messages about the execution of the application
		Info,
		// Applies to detailed informations useful when debugging
		Debug,
		// Applies to very detailed informations about the execution
		Trace
	};

	// Contract for a Logger in Stormancer.
	class ILogger
	{
	public:
		ILogger();
		virtual ~ILogger();

		// Logs a string message
		virtual void log(LogLevel level, wstring category, wstring message, wstring data) = 0;
	};
};
