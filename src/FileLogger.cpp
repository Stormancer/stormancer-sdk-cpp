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

	void FileLogger::log(const char* message)
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

	void FileLogger::log(LogLevel level, const char* category, const char* message, const char* data)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			auto ptr = format(level, category, message, data);
			_myfile << ptr.get() << std::endl;
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
