#include "stormancer.h"

namespace Stormancer
{
	FileLogger::FileLogger()
		: _fileName(nowDateStr() + ".log"),
		_myfile(_fileName, std::ofstream::out | std::ofstream::app)
	{
		tryOpenFile();
	}

	FileLogger::~FileLogger()
	{
		_myfile.close();
	}

	void FileLogger::log(std::string message)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << message << std::endl;
			_myfile.flush();
			_myfile.close();
		}
		_mutex.unlock();
	}

	void FileLogger::log(LogLevel level, std::string category, std::string message, std::string data)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << format(level, category, message, data) << std::endl;
			_myfile.flush();
			_myfile.close();
		}
		_mutex.unlock();
	}
	
	void FileLogger::log(const std::exception& e)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << formatException(e) << std::endl;
			_myfile.flush();
			_myfile.close();
		}
		_mutex.unlock();
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
				return _myfile.is_open();
			}
		}
	}
};
