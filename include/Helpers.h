#pragma once
#include "stdafx.h"

namespace Stormancer
{
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

	namespace Helpers
	{
		template<typename T>
		string mapKeys(map<string, T> map);

		template<typename T, typename U>
		bool mapContains(map<T, U> map, T& key)
		{
			return (map.find(key) != map.end) ? true : false;
		}

		string vectorJoin(vector<string> vector, string glue = "");

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
	};
};
