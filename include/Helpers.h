#pragma once
#include "headers.h"

namespace Stormancer
{
	// vector flux operators

	template<typename T>
	vector<T>& operator<<(vector<T>& v, const T& data)
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

		anyMap stringMapToAnyMap(stringMap& sm);

		template<typename TKey, typename TValue>
		vector<TKey*> mapKeysPtr(map<TKey, TValue>& map)
		{
			vector<TKey*> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(&it->first);
			}
			return vec;
		}

		template<typename TKey, typename TValue>
		vector<TKey> mapKeysCpy(map<TKey, TValue>& map)
		{
			vector<TKey> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

		template<typename TKey, typename TValue>
		vector<TValue*> mapValuesPtr(map<TKey, TValue>& map)
		{
			vector<TValue*> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(&it->second);
			}
			return vec;
		}

		template<typename TKey, typename TValue>
		vector<TValue> mapValuesCopy(map<TKey, TValue>& map)
		{
			vector<TValue> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->second);
			}
			return vec;
		}

		template<typename TKey, typename TValue>
		bool mapContains(map<TKey, TValue>& map, TKey& key)
		{
			return (map.find(key) != map.end()) ? true : false;
		}

		wstring vectorJoin(vector<wstring> vector, wstring glue = L"");

		vector<wstring> stringSplit(const wstring& str, const wstring separator);

		wstring stringTrim(wstring& str, wchar_t ch = ' ');

		template<typename T>
		inline wstring to_wstring(T data)
		{
			return to_wstring(to_string(data));
		}

		template<>
		inline wstring to_wstring<string>(string str)
		{
			return wstring(str.begin(), str.end());
			//wstring_convert<codecvt_utf8<wchar_t>, wchar_t> convert;
			//return convert.from_bytes(str);
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
			return string(str.begin(), str.end());
			//wstring_convert<codecvt_utf8<wchar_t>, wchar_t> convert;
			//return convert.to_bytes(str);
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
			StringFormat(wstring format, Args&... args)
			{
				wstring& formatTmp = format;
				int _[] = { 0, (formatTmp = replace(formatTmp, args), 0)... };
				stream << formatTmp;
			}

			template<typename T>
			wstring replace(wstring& format, T& replacement)
			{
				return replace(format, to_wstring(replacement));
			}

			template<>
			wstring replace<wstring>(wstring& format, wstring& replacement)
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
			StringFormat& operator<<(T& data)
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

		private:
			wstringstream stream;
			int formatI = 0;
		};

		pplx::task<void> taskCompleted()
		{
			pplx::task_completion_event<void> tce;
			tce.set();
			return create_task(tce);
		}

		template<typename T>
		pplx::task<T> taskCompleted(T result)
		{
			task_completion_event<T> tce;
			tce.set(result);
			return create_task(tce);
		}

		template<typename T>
		pplx::task<T> taskFromException(exception& ex)
		{
			task_completion_event<T> tce;
			tce.set_exception(ex);
			return create_task(tce);
		}

		pplx::task<void> taskIf(bool condition, function<pplx::task<void>()> action)
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

		template<typename T, typename U>
		void streamCopy(T* fromStream, U* toStream)
		{
			streamsize n = fromStream->rdbuf()->in_avail();
			char* c = new char[n];
			fromStream->readsome(c, n);
			toStream->write(c, n);
			delete[] c;
		}

		wstring nowStr();
	};
};
