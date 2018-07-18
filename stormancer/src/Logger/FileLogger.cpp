#include "stormancer/stdafx.h"
#include "stormancer/Logger/FileLogger.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	FileLogger::FileLogger(const char* filepath, bool immediate)
		: _fileName(std::strlen(filepath) ? filepath : "stormancer_" + nowDateStr() + ".log"),
		_myfile(_fileName, std::ofstream::out | std::ofstream::app),
		_immediate(immediate)
	{
		tryOpenFile();
	}

	FileLogger::~FileLogger()
	{
		_myfile.close();
	}

	void FileLogger::log(const std::string& message)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (tryOpenFile())
		{
			_myfile << message << std::endl;
			if (_immediate)
			{
				_myfile.flush();
			}
		}
	}

	void FileLogger::log(LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (tryOpenFile())
		{
			auto str = format(level, category, message,data);
			_myfile << str << std::endl;
			if (_immediate)
			{
				_myfile.flush();
			}
		}
	}
	
	void FileLogger::log(const std::exception& ex)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (tryOpenFile())
		{
			_myfile << formatException(ex) << std::endl;
			if (_immediate)
			{
				_myfile.flush();
			}
		}
	}

	bool FileLogger::tryOpenFile()
	{
		if (_myfile.is_open())
		{
			return true;
		}
		else
		{
			_myfile.open(_fileName, std::wofstream::out | std::ofstream::app);
			if (_myfile.is_open())
			{
				return true;
			}
			else
			{
				_myfile.open(_fileName, std::ofstream::out | std::ofstream::trunc);
				if (_myfile.is_open())
				{
					return true;
				}
				else
				{
					throw std::runtime_error("FileLogger can't open the file " + _fileName + " to log.");
				}
			}
		}
	}
};
