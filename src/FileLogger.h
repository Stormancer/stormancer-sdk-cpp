#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	/// Logger which writing in a file.
	class FileLogger : public ILogger
	{
	public:

		/// Constructor.
		STORMANCER_DLL_API FileLogger();

		/// Destructor.
		STORMANCER_DLL_API virtual ~FileLogger();

	public:
	
		/// A basic message log.
		/// \message The message to log.
		void log(std::string message);
		
		/// A detailed message log.
		/// \param level The level.
		/// \param category The category (typically the source).
		/// \param message The message.
		/// \param data Some extra data.
		void log(LogLevel level, std::string category, std::string message, std::string data);
		
		/// Log details about an exception.
		/// \param e The exception.
		void log(const std::exception& e);

	private:
	
		/// tries to open the log file on disk.
		bool tryOpenFile();

	private:
	
		/// Name of the log file.
		std::string _fileName;
		
		/// Stream to the log file.
		std::ofstream _myfile;
	};
};
