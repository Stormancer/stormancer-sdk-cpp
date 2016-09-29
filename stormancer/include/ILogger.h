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
		STORMANCER_DLL_API static std::shared_ptr<ILogger> instance();

		/// Set the instance of the current logger.
		/// \param logger The instance to set.
		/// \return The set instance.
		STORMANCER_DLL_API static std::shared_ptr<ILogger> instance(std::shared_ptr<ILogger> logger);

	public:

		/// Logs a simple message
		virtual void log(const std::string message) = 0;

		/// Logs a full message
		virtual void log(LogLevel level, const std::string category, const std::string message) = 0;


		/// Logs an exception
		virtual void log(const std::exception& e) = 0;

		/// A basic format of the log message.
		/// \param level The log level.
		/// \param category The category of the log (the source).
		/// \param message The message of the log.
		/// \param data Some additional data.
		/// \return The formatted message.
		STORMANCER_DLL_API static std::string format(LogLevel level, const std::string category, const std::string message);

		/// A basic format of an exception.
		/// \param e The exception.
		/// \return The formatted message.
		STORMANCER_DLL_API static std::shared_ptr<const char> formatException(const std::exception& e);

	protected:

		STORMANCER_DLL_API ILogger();

		STORMANCER_DLL_API virtual ~ILogger();

	private:
		static std::shared_ptr<ILogger> _logger;
	};
};
