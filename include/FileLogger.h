#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	class FileLogger : public ILogger
	{
	public:
		STORMANCER_DLL_API FileLogger();
		STORMANCER_DLL_API virtual ~FileLogger();

	public:
		void log(LogLevel level, wstring category, wstring message, wstring data);
		void log(const std::exception& e);

	private:
		bool tryOpenFile();

	private:
		string _fileName;
		wofstream _myfile;
	};
};
