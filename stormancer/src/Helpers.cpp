#include "stdafx.h"
#include "Helpers.h"





namespace Stormancer
{
	bool ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	std::string vectorJoin(const std::vector<std::string>& vector, const std::string& glue)
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

	std::vector<std::string> stringSplit(const std::string& str, const std::string& separator)
	{
		std::vector<std::string> splitted;
		size_t cursor = 0, lastCursor = 0;
		while ((cursor = str.find(separator, cursor)) != std::string::npos)
		{
			splitted << str.substr(lastCursor, cursor - lastCursor);
			lastCursor = cursor;
			cursor++;
		}
		splitted << str.substr(lastCursor + 1, str.length() - lastCursor);
		return splitted;
	}

	std::string stringTrim(const std::string& str2, char ch)
	{
		auto str = str2;
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

	std::vector<std::wstring> wstringSplit(const std::wstring& str, const std::wstring& separator)
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

	std::wstring wstringTrim(const std::wstring& str2, wchar_t ch)
	{
		auto str = str2;
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


	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action)
	{
		if (condition)
		{
			return action();
		}
		else
		{
			return pplx::task_from_result();
		}
	}

	pplx::task<void> taskDelay(int milliseconds, pplx::cancellation_token token)
	{
		pplx::task<void> sleepTask = pplx::task<void>([milliseconds]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		}, pplx::task_options(token));

		if (token.is_cancelable())
		{
			pplx::task_completion_event<void> tce;
			token.register_callback([tce]() { tce.set(); });
			pplx::task<void> t(tce);

			std::vector<pplx::task<void>> v{ t, sleepTask };

			return pplx::when_any(v.begin(), v.end()).then([](size_t) {});
		}
		else
		{
			return sleepTask;
		}
	}

	char* readToEnd(bytestream* stream, std::streamsize* length)
	{
		*length = stream->rdbuf()->in_avail();
		char* data = new char[(uint32)*length];
		stream->read(data, *length);
		return data;
	}

	std::time_t nowTime_t()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	std::string time_tToStr(std::time_t time, bool local)
	{
		return time_tToStr(time, (local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ"));
	}

	std::string time_tToStr(std::time_t time, const char* format)
	{
		std::stringstream ss;
#if defined(_WIN32)
		struct tm timeinfo;
		auto err = localtime_s(&timeinfo, &time);
		if (err == 0)
		{
			ss << std::put_time((tm*)&timeinfo, format);
		}
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
#if defined(_WIN32)
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

	std::string nowDateStr(bool)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%Y-%m-%d");
	}

	std::string nowTimeStr(bool)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%H:%M:%S");
	}

	std::string stringifyBytesArray(std::string bytes, bool hex)
	{
		std::stringstream ss;
		if (hex)
		{
			ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
		}
		for (std::size_t i = 0; i < bytes.length(); ++i)
		{
			if (i)
			{
				ss << ' ';
			}
			ss << std::setw(2) << (int)(byte)bytes[i];
		}
		return ss.str();
	}

	bool compareExchange(std::function<bool()> const compare, std::function<void()> exchange)
	{
		if (compare())
		{
			exchange();
			return true;
		}
		return false;
	}

	bool compareExchange(std::mutex& mutex, std::function<bool()> const compare, std::function<void()> exchange)
	{
		if (compare())
		{
			std::lock_guard<std::mutex> lg(mutex);
			if (compare())
			{
				exchange();
				return true;
			}
		}
		return false;
	}
};
