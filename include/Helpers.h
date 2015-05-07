#pragma once
#include "headers.h"
#include "basic_bytebuf.h"
#include "basic_bytestream.h"

namespace Stormancer
{
#pragma region flux

	// vector flux operators

	template<typename T>
	vector<T>& operator<<(vector<T>& v, const T& data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	vector<T>& operator<<(vector<T>& v, const T&& data)
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

	template<typename T>
	bytestream& operator<<(bytestream& bs, T& data)
	{
#ifdef _IS_BIG_ENDIAN
		T tmp(data);
		reverseByteOrder(&tmp);
		bs.write((char*)&tmp, sizeof(T));
#else
		bs.write((char*)&data, sizeof(T));
#endif
		return bs;
	}

	template<typename T>
	bytestream& operator<<(bytestream& bs, T&& data)
	{
		T tmp(data);
		return (bs << tmp);
	}

	STORMANCER_DLL_API bytestream& operator<<(bytestream& bs, const char* data);

	STORMANCER_DLL_API bytestream& operator<<(bytestream& bs, const wchar_t* data);

	template<typename T>
	bytestream& operator>>(bytestream& bs, T& data)
	{
		char* tmp = (char*)&data;
		bs.read(tmp, sizeof(T));
#ifdef _IS_BIG_ENDIAN
		reverseByteOrder(&data);
#endif
		return bs;
	}

	STORMANCER_DLL_API bytestream& operator>>(bytestream& bs, string& data);

	STORMANCER_DLL_API bytestream& operator>>(bytestream& bs, wstring& data);

	template<typename T>
	T* reverseByteOrder(T* data, size_t n = -1)
	{
		char* tmp = (char*)data;
		std::reverse(tmp, tmp + (n >= 0 ? n : sizeof(T)));
		return data;
	}

#pragma endregion

	namespace Helpers
	{
#pragma region map

		anyMap stringMapToAnyMap(stringMap& sm);

		template<typename TKey, typename TValue>
		vector<TKey> mapKeys(map<TKey, TValue>& map)
		{
			vector<TKey> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

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
		vector<TValue> mapValues(map<TKey, TValue>& map)
		{
			vector<TValue> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->second);
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
		bool mapContains(map<TKey, TValue>& map, TKey& key)
		{
			return (map.find(key) != map.end()) ? true : false;
		}

#pragma endregion

#pragma region functional

#pragma region action template

		template<typename TParam = void>
		class Action
		{
			using TFunction = function < void(TParam) > ;

		public:
			Action()
			{
			}

			Action(const Action& other)
				: _functions(other._functions)
			{
			}

			Action(Action&& right)
				: _functions(move(right._functions))
			{
			}

			virtual ~Action()
			{
				for (auto* f : _functions)
				{
					delete f;
				}
			}

		public:
			Action& operator=(TFunction* function)
			{
				_functions.clear();
				_functions.push_back(function);
				return *this;
			}

			Action& operator+=(TFunction* function)
			{
				_functions.push_back(function);
				return *this;
			}

			Action& operator-=(TFunction* function)
			{
				auto it = find(_functions.begin(), _functions.end(), function);
				if (it != _functions.end())
				{
					_functions.erase(it);
				}
			}

			Action& operator()(TParam data)
			{
				for (auto* f : _functions)
				{
					(*f)(data);
				}
				return *this;
			}

		private:
			vector<TFunction*> _functions;
		};

#pragma endregion

#pragma region void action

		template<>
		class Action < void >
		{
			using TFunction = function < void(void) > ;

		public:
			Action()
			{
			}

			Action(const Action& other)
				: _functions(other._functions)
			{
			}

			Action(Action&& right)
				: _functions(move(right._functions))
			{
			}

			virtual ~Action()
			{
				for (auto* f : _functions)
				{
					delete f;
				}
			}

		public:
			Action& operator=(TFunction* function)
			{
				_functions.clear();
				_functions.push_back(function);
				return *this;
			}

			Action& operator+=(TFunction* function)
			{
				_functions.push_back(function);
				return *this;
			}

			Action& operator-=(TFunction* function)
			{
				auto it = find(_functions.begin(), _functions.end(), function);
				if (it != _functions.end())
				{
					_functions.erase(it);
				}
			}

			Action& operator()()
			{
				for (auto* f : _functions)
				{
					(*f)();
				}
				return *this;
			}

		private:
			vector<TFunction*> _functions;
		};

#pragma endregion

#pragma endregion

#pragma region string

		wstring STORMANCER_DLL_API vectorJoin(vector<wstring> vector, wstring glue = L"");

		vector<wstring> STORMANCER_DLL_API stringSplit(const wstring& str, const wstring separator);

		wstring STORMANCER_DLL_API stringTrim(wstring& str, wchar_t ch = ' ');

		template<typename T>
		wstring to_wstring(T data)
		{
			return to_wstring(to_string(data));
		}

		template<> STORMANCER_DLL_API wstring to_wstring<string>(string str);

		template<>
		wstring to_wstring<const char*>(const char* str);

		template<typename T>
		string to_string(T& data)
		{
			return std::to_string(data);
		}

		template<>
		string to_string<wstring>(wstring& str);

		template<>
		string to_string<vector<byte>>(vector<byte>& v);

		template<typename T1, typename T2>
		T2 convert(T1& data)
		{
			return T2(data);
		}

		template<>
		vector<byte> convert<string, vector<byte>>(string& str);

		class StringFormat
		{
		public:
			STORMANCER_DLL_API StringFormat();

			template<typename... Args>
			StringFormat(wstring format, Args&... args)
			{
				int _[] = { 0, (format = replace(format, args), 0)... };
				stream << format;
			}

		public:
			STORMANCER_DLL_API wstring str();

			STORMANCER_DLL_API const wchar_t* c_str();

			STORMANCER_DLL_API operator string();
			STORMANCER_DLL_API operator wstring();

			template<typename T>
			wstring replace(wstring& format, T& replacement)
			{
				return replace(format, to_wstring(replacement));
			}

			template<>
			wstring replace<wstring>(wstring& format, wstring& replacement);

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

		private:
			wstringstream stream;
			int formatI = 0;
		};

#pragma endregion

#pragma region task

		pplx::task<void> taskCompleted();

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

		pplx::task<void> taskIf(bool condition, function<pplx::task<void>()> action);

#pragma endregion

#pragma region stream

		bytestream* convertRakNetPacketToStream(RakNet::Packet* packet);

		void deleteStringBuf(stringbuf* sb);

		template<typename T, typename U>
		void streamCopy(T* fromStream, U* toStream)
		{
			uint32 n = static_cast<uint32>(fromStream->rdbuf()->in_avail());
			char* c = new char[n];
			fromStream->readsome(c, n);
			toStream->write(c, n);
			delete[] c;
		}

#pragma endregion

#pragma region time

		time_t STORMANCER_DLL_API nowTime_t();
		wstring STORMANCER_DLL_API time_tToStr(time_t& now, bool local = false);
		wstring STORMANCER_DLL_API nowStr(bool local = false);

#pragma endregion

#pragma region other

		bool ensureSuccessStatusCode(int statusCode);

		void displayException(const exception& e);

#pragma endregion

		template<typename T>
		uint64 ptrToUint64(T* ptr)
		{
			return *static_cast<uint64*>(static_cast<void*>(ptr));
		}
	};
};
