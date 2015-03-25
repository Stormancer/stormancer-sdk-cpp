#include "stormancer.h"

namespace Stormancer
{
	bool Helpers::ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	string Helpers::vectorJoin(vector<string> vector, string glue)
	{
		stringstream ss;
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

	vector<string> Helpers::stringSplit(const string& str, const string& glue)
	{
		vector<string> splitted;
		size_t cursor = 0, lastCursor = 0;
		while ((cursor = str.find(glue, cursor)) != string::npos)
		{
			splitted << str.substr(lastCursor, cursor - 1 - lastCursor);
			lastCursor = cursor;
			cursor++;
		}
		splitted << str.substr(lastCursor, str.length() - 1 - lastCursor);
		return splitted;
	}

	Helpers::StringFormat::StringFormat()
	{
	}

	string Helpers::StringFormat::str()
	{
		return stream.str();
	}

	/*Helpers::StringFormat::operator string()
	{
		return str();
	}*/

	Helpers::StringFormat::operator const char*()
	{
		return stream.str().c_str();
	}

	string Helpers::stringTrim(string& str)
	{
		// TODO
		return str;
	};

	wstring Helpers::to_wstring(string& str)
	{
		return wstring(str.begin(), str.end());
	}
};
