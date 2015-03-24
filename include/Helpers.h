#pragma once
#include "headers.h"

namespace Stormancer
{
	// vector flux operators

	template<typename T>
	vector<T>& operator<<(vector<T>& v, const T data);

	template<typename T>
	vector<T>& operator>>(vector<T>& v, T& data);

	namespace Helpers
	{
		bool ensureSuccessStatusCode(int statusCode);

		template<typename T>
		string mapKeys(map<string, T> map);

		template<typename T, typename U>
		bool mapContains(map<T, U> map, T& key);

		string vectorJoin(vector<string> vector, string glue = "");

		vector<string> stringSplit(const string& str, const string& glue);

		class StringFormat
		{
		public:
			StringFormat();
			StringFormat(string format, initializer_list<string> args);

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
		string stringFormat(string format, initializer_list<string> args);

		string stringTrim(string str);
	};
};
