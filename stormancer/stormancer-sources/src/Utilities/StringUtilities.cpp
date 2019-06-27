#include "stormancer/stdafx.h"
#include "stormancer/Utilities/StringUtilities.h"
#include <sstream>

namespace Stormancer
{
	std::string vectorJoin(const std::vector<std::string>& vector, const std::string& glue)
	{
		std::stringstream ss;
		for (size_t i = 0; i < vector.size(); i++)
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
		size_t cursor = 0, lastCursor = 0, separatorSize = separator.size();
		while ((cursor = str.find(separator, cursor)) != std::string::npos)
		{
			splitted.emplace_back(str.substr(lastCursor, cursor - lastCursor));
			cursor += separatorSize;
			lastCursor = cursor;
		}
		splitted.emplace_back(str.substr(lastCursor, str.length() - lastCursor));
		return splitted;
	}

	std::vector<std::string> stringSplit(const std::string& str, const char separator)
	{
		return stringSplit(str, std::string(&separator, 1));
	}

	std::string stringTrim(const std::string& str2, char ch)
	{
		auto str = str2;
		std::function<int(int)> ischar = [ch](int c) { return ((c == ch) ? 1 : 0); };
		str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ischar)));
		str.erase(find_if(str.rbegin(), str.rend(), not1(ischar)).base(), str.end());
		return str;
	}

	std::vector<std::wstring> wstringSplit(const std::wstring& str, const std::wstring& separator)
	{
		std::vector<std::wstring> splitted;
		size_t cursor = 0, lastCursor = 0, separatorSize = separator.size();
		while ((cursor = str.find(separator, cursor)) != std::wstring::npos)
		{
			splitted.emplace_back(str.substr(lastCursor, cursor - lastCursor));
			cursor += separatorSize;
			lastCursor = cursor;
		}
		splitted.emplace_back(str.substr(lastCursor, str.length() - lastCursor));
		return splitted;
	}

	std::vector<std::wstring> wstringSplit(const std::wstring& str, const wchar_t separator)
	{
		return wstringSplit(str, std::wstring(&separator, 1));
	}

	std::wstring wstringTrim(const std::wstring& str2, wchar_t ch)
	{
		auto str = str2;
		std::function<int(int)> ischar = [ch](int c) { return ((c == ch) ? 1 : 0); };
		str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ischar)));
		str.erase(find_if(str.rbegin(), str.rend(), not1(ischar)).base(), str.end());
		return str;
	}
}
