#include "stormancer.h"

namespace Stormancer
{
	bool Helpers::ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	anyMap Helpers::stringMapToAnyMap(stringMap& sm)
	{
		anyMap am;
		for (auto it = sm.begin(); it != sm.end(); ++it)
		{
			am[it->first] = (void*)&it->second;
		}
		return am;
	}

	std::string Helpers::vectorJoin(std::vector<std::string>& vector, std::string glue)
	{
		std::stringstream ss;
		for (size_t i = 0; i < vector.size(); ++i)
		{
			if (i != 0)
			{
				ss << glue;
			}
			ss << vector[i];
		}
		return ss.str();
	}

	std::vector<std::wstring> Helpers::stringSplit(const std::wstring& str, const std::wstring separator)
	{
		std::vector<std::wstring> splitted;
		size_t cursor = 0, lastCursor = 0;
		while ((cursor = str.find(separator, cursor)) != std::wstring::npos)
		{
			splitted << str.substr(lastCursor, cursor - lastCursor);
			lastCursor = cursor;
			cursor++;
		}
		splitted << str.substr(lastCursor + 1, str.length() - lastCursor);
		return splitted;
	}

	std::wstring Helpers::stringTrim(std::wstring& str, wchar_t ch)
	{
		std::function<int(int)> ischar = [ch](int c) -> int {
			if (c == ch)
			{
				return 1;
			}
			return 0;
		};
		str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ischar)));
		str.erase(find_if(str.rbegin(), str.rend(), not1(ischar)).base(), str.end());
		return str;
	};

	pplx::task<void> Helpers::taskCompleted()
	{
		pplx::task_completion_event<void> tce;
		tce.set();
		return create_task(tce);
	}

	pplx::task<void> Helpers::taskIf(bool condition, std::function<pplx::task<void>()> action)
	{
		if (condition)
		{
			return action();
		}
		else
		{
			return taskCompleted();
		}
	}

	time_t Helpers::nowTime_t()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	std::string Helpers::time_tToStr(time_t& time, bool local)
	{
		return time_tToStr(time, (local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ"));
	}

	std::string Helpers::time_tToStr(time_t& time, const char* format)
	{
		struct tm timeinfo;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&timeinfo, &time);
#else
		localtime_r(&time, &timeinfo); // POSIX
#endif
		std::stringstream ss;
		ss << std::put_time((tm*)&timeinfo, format);
		return ss.str();
	}

	std::string Helpers::nowStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, local);
	}

	std::string Helpers::nowStr(const char* format)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, format);
	}

	std::string Helpers::nowDateStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%Y-%m-%d");
	}

	std::string Helpers::nowTimeStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%H:%M:%S");
	}

	bytestream& operator<<(bytestream& bs, const char* data)
	{
		bs.write((char*)data, strlen(data));
		return bs;
	}

	bytestream& operator<<(bytestream& bs, const std::string& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator<<(bytestream& bs, std::string& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator<<(bytestream& bs, const wchar_t* data)
	{
		bs.write((char*)data, 2 * wcslen(data));
		return bs;
	}

	bytestream& operator<<(bytestream& bs, const std::wstring& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator<<(bytestream& bs, std::wstring& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator>>(bytestream& bs, std::string& str)
	{
		uint32 len = (uint32)bs.rdbuf()->in_avail();
		str.reserve(len);
		char* data = new char[len];
		bs.read(data, len);
		str.assign(data, len);
		delete[] data;
		return bs;
	}

	bytestream& operator>>(bytestream& bs, std::wstring& str)
	{
		uint32 len = (uint32)bs.rdbuf()->in_avail();
		uint32 nbChars = len / 2;
		str.reserve(nbChars);
		char* data = new char[len];
		bs.read(data, len);
		str.assign((wchar_t*)data, nbChars);
		delete[] data;
		return bs;
	}
};
