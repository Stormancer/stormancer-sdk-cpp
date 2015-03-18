#pragma once
#include "stdafx.h"

namespace Stormancer
{
	// The available log levels.
	enum LogLevel
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
		virtual void Log(LogLevel level, string category, string message, string data) = 0;
	};
};
