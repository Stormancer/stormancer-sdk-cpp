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

		void displayException(const exception& e);

		template<typename T>
		wstring mapKeys(map<wstring, T> map)
		{
			auto vec = vector<wstring>();
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

		wstring vectorJoin(vector<wstring> vector, wstring glue = L"");

		vector<wstring> stringSplit(const wstring& str, const wstring glue);

		wstring stringTrim(wstring str, wchar_t ch = ' ');

		template<typename T>
		inline wstring to_wstring(T data)
		{
			return to_wstring(to_string(data));
		}

		template<>
		inline wstring to_wstring<string>(string str)
		{
			wstring_convert<codecvt_utf8<wchar_t>, wchar_t> convert;
			return convert.from_bytes(str);
		}

		template<>
		inline wstring to_wstring<const char*>(const char* str)
		{
			return to_wstring(string(str));
		}

		template<typename T>
		inline string to_string(T& data)
		{
			return std::to_string(data);
		}

		template<>
		inline string to_string<wstring>(wstring& str)
		{
			wstring_convert<codecvt_utf8<wchar_t>, wchar_t> convert;
			return convert.to_bytes(str);
		}

		template<>
		inline string to_string<vector<byte>>(vector<byte>& v)
		{
			string str;
			str.resize(v.size());
			for (size_t i = 0; i < v.size(); i++)
			{
				str[i] = v[i];
			}
			return str;
		}

		class StringFormat
		{
		public:
			StringFormat();

			template<typename... Args>
			StringFormat(wstring format, Args... args)
			{
				int _[] = { 0, (format = replace(format, args), 0)... };
				stream << format;
			}

			template<typename T>
			wstring replace(wstring format, T replacement)
			{
				return replace(format, to_wstring(replacement));
			}

			template<>
			wstring replace<wstring>(wstring format, wstring replacement)
			{
				wstring toFind = L"{" + to_wstring(formatI) + L"}";
				wstring::size_type start = format.find(toFind);
				if (start != wstring::npos)
				{
					wstring::size_type size = toFind.size();
					format.replace(start, size, replacement);
				}
				formatI++;
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

			wstring str();

			const wchar_t* c_str();

			operator string();
			operator wstring();
			//operator const char*();
			//operator const wchar_t*();

		private:
			wstringstream stream;
			int formatI = 0;
		};
	};
};
