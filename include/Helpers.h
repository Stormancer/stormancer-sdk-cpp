#pragma once
#include "headers.h"

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
	vector<T>& operator>>(vector<T>& v, T& data)
	{
		data = v.pop_back();
		return v;
	}

#pragma endregion

	namespace Helpers
	{
#pragma region map

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

#pragma endregion

#pragma region functional

		template<typename TData>
		void vectorExec(vector<function<void(TData*)>>& v, TData* data)
		{
			for (uint32 i = 0; i < v.size(); i++)
			{
				v[i](data);
			}
		}

#pragma endregion

#pragma region string

		wstring vectorJoin(vector<wstring> vector, wstring glue = L"");

		vector<wstring> stringSplit(const wstring& str, const wstring separator);

		wstring stringTrim(wstring& str, wchar_t ch = ' ');

		template<typename T>
		wstring to_wstring(T data)
		{
			return to_wstring(to_string(data));
		}

		template<>
		wstring to_wstring<string>(string str);

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

		// Template conversion function

		template<typename T1, typename T2>
		T2 convert(T1& data)
		{
			return T2(T1);
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

			STORMANCER_DLL_API wstring str();

			STORMANCER_DLL_API const wchar_t* c_str();

			STORMANCER_DLL_API operator string();
			STORMANCER_DLL_API operator wstring();

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

		//bytestream* convertRakNetBitStreamToByteStream(RakNet::BitStream* stream);

		stringstream* convertRakNetPacketToStringStream(RakNet::Packet* packet, RakNet::RakPeerInterface* peer = nullptr);

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

		time_t nowTime_t();
		wstring time_tToStr(time_t& now, bool local = false);
		wstring nowStr(bool local = false);

#pragma endregion

#pragma region other

		bool ensureSuccessStatusCode(int statusCode);

		void displayException(const exception& e);

#pragma endregion
	};
};
