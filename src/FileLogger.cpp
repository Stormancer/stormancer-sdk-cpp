#include "stormancer.h"

namespace Stormancer
{
	FileLogger::FileLogger()
		: _fileName(Helpers::to_string(Helpers::nowDateStr()) + ".log")
	{
		tryOpenFile();
	}

	FileLogger::~FileLogger()
	{
	}

	void FileLogger::log(LogLevel level, wstring category, wstring message, wstring data)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << format(level, category, message, data);
			_myfile.close();
		}
		_mutex.unlock();
	}
	
	void FileLogger::log(const std::exception& e)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << formatException(e);
			_myfile.close();
		}
		_mutex.unlock();
	}

	bool FileLogger::tryOpenFile()
	{
		if (_myfile.good())
		{
			return true;
		}
		else
		{
			_myfile.open(_fileName, ios::out | ios::app);
			if (_myfile.good())
			{
				return true;
			}
			else
			{
				_myfile.open(_fileName, ios::out | ios::trunc);
				return _myfile.good();
			}
		}
	}
};
