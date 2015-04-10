#include "stormancer.h"

namespace Stormancer
{
	bool Helpers::ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	void Helpers::displayException(const exception& e)
	{
		string msg = e.what();
		wcout << Helpers::StringFormat(L"Error exception: {0}", msg).c_str() << endl;
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

	Helpers::StringFormat::StringFormat()
	{
	}

	wstring Helpers::StringFormat::str()
	{
		return stream.str();
	}

	const wchar_t* Helpers::StringFormat::c_str()
	{
		return stream.str().c_str();
	}

	Helpers::StringFormat::operator string()
	{
		return to_string(stream.str());
	}

	Helpers::StringFormat::operator wstring()
	{
		return stream.str();
	}

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

	time_t nowTime_t()
	{
		return chrono::system_clock::to_time_t(chrono::system_clock::now());
	}

	wstring Helpers::nowStr()
	{
		time_t now = Helpers::nowTime_t();
		return Helpers::time_tToStr(now);
	}

	wstring Helpers::time_tToStr(time_t now)
	{
		struct tm timeinfo;
		localtime_s(&timeinfo, &now);
		stringstream ss;
		ss << put_time(&timeinfo, "%F %T");
		return Helpers::to_wstring(ss.str());
	}
};
