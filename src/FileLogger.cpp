#include "stormancer.h"

namespace Stormancer
{
	FileLogger::FileLogger()
		: _fileName(Helpers::to_string(Helpers::nowDateStr()) + ".log"),
		_myfile(_fileName, wofstream::out | wofstream::app)
	{
		tryOpenFile();
	}

	FileLogger::~FileLogger()
	{
		_myfile.close();
	}

	void FileLogger::log(wstring message)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << message << endl;
			_myfile.flush();
			_myfile.close();
		}
		_mutex.unlock();
	}

	void FileLogger::log(LogLevel level, wstring category, wstring message, wstring data)
	{
		_mutex.lock();
		if (tryOpenFile())
		{
			_myfile << format(level, category, message, data) << endl;
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
			_myfile << formatException(e) << endl;
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
			_myfile.open(_fileName, wofstream::out | wofstream::app);
			if (_myfile.is_open())
			{
				return true;
			}
			else
			{
				_myfile.open(_fileName, wofstream::out | wofstream::trunc);
				return _myfile.is_open();
			}
		}
	}
};
