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

	wstring Helpers::vectorJoin(vector<wstring> vector, wstring glue)
	{
		wstringstream ss;
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

	vector<wstring> Helpers::stringSplit(const wstring& str, const wstring separator)
	{
		vector<wstring> splitted;
		size_t cursor = 0, lastCursor = 0;
		while ((cursor = str.find(separator, cursor)) != wstring::npos)
		{
			splitted << str.substr(lastCursor, cursor - lastCursor);
			lastCursor = cursor;
			cursor++;
		}
		splitted << str.substr(lastCursor + 1, str.length() - lastCursor);
		return splitted;
	}

	wstring Helpers::stringTrim(wstring& str, wchar_t ch)
	{
		function<int(int)> ischar = [ch](int c) -> int {
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

	pplx::task<void> Helpers::taskIf(bool condition, function<pplx::task<void>()> action)
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
		return chrono::system_clock::to_time_t(chrono::system_clock::now());
	}

	wstring Helpers::time_tToStr(time_t& time, bool local)
	{
		return time_tToStr(time, string(local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ"));
	}

	wstring Helpers::time_tToStr(time_t& time, string format)
	{
		struct tm timeinfo;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&timeinfo, &time);
#else
		localtime_r(&time, &timeinfo); // POSIX
#endif
		stringstream ss;
		ss << put_time(const_cast<tm*>(&timeinfo), format.c_str());
		return Helpers::to_wstring(ss.str());
	}

	wstring Helpers::nowStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, local);
	}

	wstring Helpers::nowStr(string format)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, format);
	}

	wstring Helpers::nowDateStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%Y-%m-%d");
	}

	wstring Helpers::nowTimeStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%H:%M:%S");
	}

	bytestream& operator<<(bytestream& bs, const char* data)
	{
		bs.write((char*)data, strlen(data));
		return bs;
	}

	bytestream& operator<<(bytestream& bs, string& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator<<(bytestream& bs, const wchar_t* data)
	{
		bs.write((char*)data, 2 * wcslen(data));
		return bs;
	}

	bytestream& operator<<(bytestream& bs, wstring& data)
	{
		return bs << data.c_str();
	}

	bytestream& operator>>(bytestream& bs, string& str)
	{
		uint32 len = (uint32)bs.rdbuf()->in_avail();
		str.reserve(len);
		char* data = new char[len];
		bs.read(data, len);
		str.assign(data, len);
		delete[] data;
		return bs;
	}

	bytestream& operator>>(bytestream& bs, wstring& str)
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

	wstring Helpers::to_wstring(const char* str)
	{
		return to_wstring(string(str));
	}

	wstring Helpers::to_wstring(string str)
	{
		return wstring(str.begin(), str.end());
	}

	string Helpers::to_string(wstring& str)
	{
		return string(str.begin(), str.end());
	}

	string Helpers::to_string(vector<byte>& v)
	{
		string str;
		str.resize(v.size());
		for (size_t i = 0; i < v.size(); i++)
		{
			str[i] = v[i];
		}
		return str;
	}

	template<>
	vector<byte> Helpers::convert<string, vector<byte>>(string& str)
	{
		vector<byte> v;
		v.resize(str.size());
		for (uint32 i = 0; v.size(); i++)
		{
			v[i] = str[i];
		}
		return v;
	}
};
