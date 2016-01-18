#include "stormancer.h"

namespace Stormancer
{
	FileLogger::FileLogger(bool immediate)
		: _fileName(nowDateStr() + ".log"),
		_myfile(_fileName, std::ofstream::out | std::ofstream::app),
		_immediate(immediate)
	{
		tryOpenFile();
	}

	FileLogger::~FileLogger()
	{
		_myfile.close();
	}

	void FileLogger::log(const char* message)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (tryOpenFile())
		{
			_myfile << message << std::endl;
			if (_immediate)
			{
				_myfile.flush();
				_myfile.close();
			}
		}
	}

	void FileLogger::log(LogLevel level, const char* category, const char* message, const char* data)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (tryOpenFile())
		{
			auto ptr = format(level, category, message, data);
			_myfile << ptr.get() << std::endl;
			if (_immediate)
			{
				_myfile.flush();
				_myfile.close();
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
				_myfile.close();
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
