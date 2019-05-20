#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Logger/ILogger.h"
#include <mutex>
#include <fstream>

namespace Stormancer
{
	/// Logger which writing in a file.
	class FileLogger : public ILogger
	{
	private:
		enum class FileLoggerMode
		{
			Deferred = 0,
			Immediate = 1
		};

	public:

		/// Constructor.
		STORMANCER_DLL_API FileLogger(const char* filepath = "", bool immediate = false);

		/// Destructor.
		STORMANCER_DLL_API virtual ~FileLogger();

	public:
	
		/// A basic message log.
		/// \message The message to log.
		void log(const std::string& message) override;
		
		/// A detailed message log.
		/// \param level The level.
		/// \param category The category (typically the source).
		/// \param message The message.
		/// \param data Some extra data.
		void log(LogLevel level, const std::string& category, const std::string& message, const std::string& data) override;
		
		/// Log details about an exception.
		/// \param e The exception.
		void log(const std::exception& ex) override;

	private:
	
		/// tries to open the log file on disk.
		bool tryOpenFile();

	private:

		std::mutex _mutex;

		/// Name of the log file.
		std::string _fileName;
		
		/// Stream to the log file.
		std::ofstream _myfile;

		/// immediate mode
		bool _immediate = false;
	};
};
