#pragma once
#include "headers.h"

namespace Stormancer
{
	// The available log levels.
	enum class LogLevel
	{
		// Applies to critical errors that prevent the program from continuing.
		Fatal = 0,
		// Applies to non critical errors
		Error = 1,
		// Applies to warnings
		Warn = 2,
		// Applies to information messages about the execution of the application
		Info = 3,
		// Applies to detailed informations useful when debugging
		Debug = 4,
		// Applies to very detailed informations about the execution
		Trace = 5
	};

	// Contract for a Logger in Stormancer.
	class ILogger
	{
	public:
		STORMANCER_DLL_API static ILogger* instance();
		STORMANCER_DLL_API static ILogger* instance(ILogger* logger);

	public:
		// Logs a string message
		virtual void log(wstring message);
		virtual void log(LogLevel level, wstring category, wstring message, wstring data);
		virtual void log(const std::exception& e);

		STORMANCER_DLL_API virtual wstring format(LogLevel level, wstring& category, wstring& message, wstring& data);
		STORMANCER_DLL_API virtual wstring formatException(const std::exception& e);

	protected:
		STORMANCER_DLL_API ILogger();
		STORMANCER_DLL_API virtual ~ILogger();

	protected:
		mutex _mutex;

	private:
		static ILogger* _logger;
	};

	using NullLogger = ILogger;
};
