#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Available log levels.
	enum class LogLevel
	{
		/// Critical error that prevent the program from continuing.
		Fatal = 0,
		/// Non critical errors
		Error = 1,
		/// Warning
		Warn = 2,
		/// Information message about the execution of the application
		Info = 3,
		/// Applies to detailed informations useful when debugging
		Debug = 4,
		/// Very detailed informations about the execution
		Trace = 5
	};

	/// Interface for the loggers.
	class ILogger
	{
	public:

		/// Get the instance of the current logger.
		STORMANCER_DLL_API static ILogger* instance();

		/// Set the instance of the current logger.
		/// \param logger The instance to set.
		/// \return The set instance.
		STORMANCER_DLL_API static ILogger* instance(ILogger* logger);

	public:
		// Logs a string message
		virtual void log(std::string message) = 0;
		virtual void log(LogLevel level, std::string category, std::string message, std::string data) = 0;
		virtual void log(const std::exception& e) = 0;

		/// A basic format of the log message.
		/// \param level The log level.
		/// \param category The category of the log (the source).
		/// \param message The message of the log.
		/// \param data Some additional data.
		/// \return The formatted message.
		STORMANCER_DLL_API static std::string format(LogLevel level, std::string& category, std::string& message, std::string& data);

		/// A basic format of an exception.
		/// \param e The exception.
		/// \return The formatted message.
		STORMANCER_DLL_API static std::string formatException(const std::exception& e);

	protected:

		STORMANCER_DLL_API ILogger();

		STORMANCER_DLL_API virtual ~ILogger();

	protected:

		std::mutex _mutex;

	private:
		static ILogger* _logger;
	};
};
