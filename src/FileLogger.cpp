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
		if (tryOpenFile())
		{
			wstring str = format(level, category, message, data);
			_myfile << str;
			_myfile.close();
		}
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
