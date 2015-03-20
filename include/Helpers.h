#pragma once
#include "libs.h"

namespace Stormancer
{
	namespace Helpers
	{
		template<typename T>
		string mapKeys(map<string, T> map);

		template<typename T, typename U>
		bool mapContains(map<T, U> map, T& key);

		string vectorJoin(vector<string> vector, string glue = "");

		vector<string> stringSplit(const string& str, const string& glue)
		{
			vector<string> splitted;
			int cursor = 0, lastCursor = 0;
			while ((cursor = str.find(glue, cursor)) != string::npos)
			{
				splitted << str.substr(lastCursor, cursor - 1 - lastCursor);
				lastCursor = cursor;
				cursor++;
			}
			splitted << str.substr(lastCursor, str.length() - 1 - lastCursor);
			return splitted;
		}

		class StringFormat
		{
		public:
			StringFormat() {}

			template<typename T>
			StringFormat& operator<<(T data);

			template<typename T>
			void operator>>(T& data);

			string str() const;

			operator string() const;
			operator const char*() const;

			stringstream stream;
		};

		template<typename T>
		string stringFormat(string format, initializer_list<string> args)
		{
			string regex = "{";
			for (int i = 0; i < args.size(); i++)
			{
				string tmp();
				tmp << "{" << to_string(i) << "}";
				replace(format.begin(), format.end(), tmp, args[i]);
			}
			return format;
		}

		string stringTrim(string str)
		{
			// TODO
			return str;
		};
	};

	// vector flux operators

	template<typename T, typename U>
	vector<T>& operator<<(const vector<T>& v, const U data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T, typename U>
	vector<T>& operator>>(const vector<T>& v, U& data)
	{
		data = v.pop_back();
		return v;
	}
};
