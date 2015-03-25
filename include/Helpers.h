#pragma once
#include "headers.h"

namespace Stormancer
{
	// vector flux operators

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

	namespace Helpers
	{
		bool ensureSuccessStatusCode(int statusCode);

		template<typename T>
		string mapKeys(map<string, T> map)
		{
			auto vec = vector<string>();
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

		template<typename T, typename U>
		bool mapContains(map<T, U> map, T& key)
		{
			return (map.find(key) != map.end) ? true : false;
		}

		string vectorJoin(vector<string> vector, string glue = "");

		vector<string> stringSplit(const string& str, const string& glue);

		class StringFormat
		{
		public:
			StringFormat();

			template<typename... Args>
			StringFormat(string format, Args... args)
			{
				int _[] = {0, (replace(format, args), 0)...};
				stream << format;
			}

			template<typename T>
			string& replace(string& format, T replacement)
			{
				return replace(format, to_string(replacement));
			}

			template<>
			string& replace<string>(string& format, string replacement)
			{
				string toFind = "{" + to_string(formatI) + "}";
				string::size_type start = format.find(toFind);
				if (start != string::npos)
				{
					string::size_type end = start + toFind.size();
					format.replace(start, end, replacement);
				}
				return format;
			}

			template<typename T>
			StringFormat& operator<<(T data)
			{
				stream << data;
				return *this;
			}

			template<typename T>
			void operator>>(T& data)
			{
				stream >> data;
			}

			string str();

			//operator string();
			operator const char*();

		private:
			stringstream stream;
			int formatI = 0;
		};

		//template<typename T>
		//string stringFormat(string format, initializer_list<string> args);

		string stringTrim(string& str);
		wstring to_wstring(string& str);
	};
};
