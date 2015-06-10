#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Enumeration of the available log levels.
	enum class LogLevel
	{
		Fatal = 0,	/// Critical error that prevent the program from continuing.
		Error = 1,	/// Non critical errors
		Warn = 2,	/// Warning
		Info = 3,	/// Information message about the execution of the application
		Debug = 4,	/// Applies to detailed informations useful when debugging
		Trace = 5	/// Very detailed informations about the execution
	};

	/// Interface for the loggers.
	class ILogger
	{
	public:

		/*! Get the current instance of the logger.
		\return The instance of the current logger.
		*/
		STORMANCER_DLL_API static ILogger* instance();

		/*! Set the instance of the current logger.
		\param logger The instance of the logger to set as the current logger.
		\return The set instance of the current logger.
		*/
		STORMANCER_DLL_API static ILogger* instance(ILogger* logger);

	public:
		// Logs a string message
		virtual void log(wstring message) = 0;
		virtual void log(LogLevel level, wstring category, wstring message, wstring data) = 0;
		virtual void log(const std::exception& e) = 0;

		/*! A basic format of the log message.
		\param level The log level.
		\param category The category of the log (the source).
		\param message The message of the log.
		\param data Some additional data.
		\return The formated message.
		*/
		STORMANCER_DLL_API static wstring format(LogLevel level, wstring& category, wstring& message, wstring& data);

		/*! A basic format of an exception.
		\param e The exception.
		\return The formated message.
		*/
		STORMANCER_DLL_API static wstring formatException(const std::exception& e);

	protected:

		/*! Constructor.
		*/
		STORMANCER_DLL_API ILogger();

		/*! Destructor.
		*/
		STORMANCER_DLL_API virtual ~ILogger();

	protected:

		/*! The mutex for using the logger.
		*/
		mutex _mutex;

	private:
		static ILogger* _logger;
	};
};
