#include "stormancer.h"

namespace Stormancer
{
	bool ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	anyMap stringMapToAnyMap(stringMap& sm)
	{
		anyMap am;
		for (auto it = sm.begin(); it != sm.end(); ++it)
		{
			am[it->first] = (void*)&it->second;
		}
		return am;
	}

	std::string vectorJoin(std::vector<std::string>& vector, std::string glue)
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

	std::vector<std::wstring> stringSplit(const std::wstring& str, const std::wstring separator)
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

	std::wstring stringTrim(std::wstring& str, wchar_t ch)
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

	pplx::task<void> taskCompleted()
	{
		pplx::task_completion_event<void> tce;
		tce.set();
		return create_task(tce);
	}

	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action)
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

	char* readToEnd(bytestream* stream, std::streamsize* length)
	{
		*length = stream->rdbuf()->in_avail();
		char* data = new char[(uint32)*length];
		stream->read(data, *length);
		return data;
	}

	time_t nowTime_t()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	std::string time_tToStr(time_t& time, bool local)
	{
		return time_tToStr(time, (local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ"));
	}

	std::string time_tToStr(time_t& time, const char* format)
	{
		std::stringstream ss;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		struct tm timeinfo;
		localtime_s(&timeinfo, &time);
		ss << std::put_time((tm*)&timeinfo, format);
#else
//		struct tm timeinfo;
//		localtime_r(&time, &timeinfo); // POSIX
//		ss << std::put_time((tm*)&timeinfo, format); // put_time not implemented in libstdc++4.9
		writetime(ss, time, format);
#endif
		return ss.str();
	}

	void writetime(std::ostream &os, std::time_t time, const char* format)
	{
		std::locale loc;
		struct tm timeinfo;
		const std::time_put<char>& tmput = std::use_facet<std::time_put<char>>(loc);
		//std::tm* now = std::localtime(&time);
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&timeinfo, &time);
#else
		localtime_r(&time, &timeinfo); // POSIX
#endif
		tmput.put(os, os, ' ', &timeinfo, format, format + strlen(format));
	}

	std::string nowStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, local);
	}

	std::string nowStr(const char* format)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, format);
	}

	std::string nowDateStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%Y-%m-%d");
	}

	std::string nowTimeStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%H:%M:%S");
	}
};
