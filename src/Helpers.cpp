#include "stormancer.h"

namespace Stormancer
{
	template<typename T>
	vector<T>& operator<<(vector<T>& v, const T data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	vector<T>& operator>>(vector<T>& v, T& data)
	{
		data = v.pop_back();
		return v;
	}

	bool Helpers::ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	template<typename T>
	vector<string> Helpers::mapKeys(map<string, T> map)
	{
		auto vec = vector<string>();
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			vec.push_back(it->first);
		}
		return vec;
	}

	template<typename T, typename U>
	bool Helpers::mapContains(map<T, U> map, T& key)
	{
		return (map.find(key) != map.end) ? true : false;
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

	Helpers::StringFormat::StringFormat(string format, initializer_list<string> args)
	{
		int i = 0;
		for (auto it = args.begin(); it != args.end(); ++it)
		{
			string tmp = "{" + to_string(i) + "}";
			std::replace(format.begin(), format.end(), tmp, (*it));
			i++;
		}
	}

	template<typename T>
	StringFormat& Helpers::StringFormat::operator << (T data)
	{
		stream << data;
		return *this;
	}

	template<typename T>
	void Helpers::StringFormat::operator >> (T& data)
	{
		stream >> data;
	}

	string Helpers::StringFormat::str() const
	{
		return stream.str();
	}

	Helpers::StringFormat::operator string() const
	{
		return stream.str();
	}

	Helpers::StringFormat::operator const char*() const
	{
		return stream.str().c_str();
	}

	string Helpers::stringTrim(string str)
	{
		// TODO
		return str;
	};
};
