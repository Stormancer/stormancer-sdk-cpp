#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	/*! A Logger class which writes in a file.
	*/
	class FileLogger : public ILogger
	{
	public:

		/*! Constructor.
		*/
		STORMANCER_DLL_API FileLogger();

		/*! Destructor.
		*/
		STORMANCER_DLL_API virtual ~FileLogger();

	public:
		void log(wstring message);
		void log(LogLevel level, wstring category, wstring message, wstring data);
		void log(const std::exception& e);

	private:
		bool tryOpenFile();

	private:
		string _fileName;
		wofstream _myfile;
	};
};
