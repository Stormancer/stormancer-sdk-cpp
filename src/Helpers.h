#pragma once
#include "headers.h"
#include "basic_bytebuf.h"
#include "basic_bytestream.h"

namespace Stormancer
{
#pragma region flux

	// vector flux operators
	
	template<typename T>
	std::vector<T>& operator<<(std::vector<T>& v, const T& data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	std::vector<T>& operator<<(std::vector<T>& v, const T&& data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	std::vector<T>& operator>>(std::vector<T>& v, T& data)
	{
		data = v.pop_back();
		return v;
	}

	template<typename T>
	std::string to_string(T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	template<typename T>
	T* reverseByteOrder(T* data, size_t n = -1)
	{
		char* tmp = (char*)data;
		std::reverse(tmp, tmp + (n < 0 ? sizeof(T) : n));
		return data;
	}

#pragma endregion

#pragma region map

		anyMap stringMapToAnyMap(stringMap& sm);

		template<typename TKey, typename TValue>
		std::vector<TKey> mapKeys(std::map<TKey, TValue>& map)
		{
			std::vector<TKey> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

		template<typename TKey, typename TValue>
		std::vector<TValue> mapValues(std::map<TKey, TValue>& map)
		{
			std::vector<TValue> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->second);
			}
			return vec;
		}

		/// Returns a boolean indicating if the map contains the key.
		template<typename TKey, typename TValue>
		bool mapContains(std::map<TKey, TValue>& map, TKey& key)
		{
			return (map.find(key) != map.end()) ? true : false;
		}

#pragma endregion

#pragma region string
		
		/// Join a string vector by using a glue string.
		/// \param vector The vector containing the strings to join.
		/// \param glue A glue string. Default is an empty string.
		STORMANCER_DLL_API std::string vectorJoin(std::vector<std::string>& vector, std::string glue = "");

		/// Split a string to a vector by using a separator string.
		/// \param str The string to split.
		/// \param separator the separator to detect in the string.
		STORMANCER_DLL_API std::vector<std::wstring> stringSplit(const std::wstring& str, const std::wstring separator);

		/// Trim a specific character from a string.
		/// \param str The string to trim.
		/// \param ch the character to remove from the string. Default is space.
		STORMANCER_DLL_API std::wstring stringTrim(std::wstring& str, wchar_t ch = ' ');

#pragma endregion

#pragma region task

		pplx::task<void> taskCompleted();

		template<typename T>
		pplx::task<T> taskCompleted(T result)
		{
			pplx::task_completion_event<T> tce;
			tce.set(result);
			return create_task(tce);
		}

		template<typename T>
		pplx::task<T> taskFromException(const std::exception& e)
		{
			pplx::task_completion_event<T> tce;
			tce.set_exception<std::exception>(e);
			return create_task(tce);
		}

		pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

#pragma endregion

#pragma region stream

		//bytestream* convertRakNetPacketToStream(RakNet::Packet* packet);

		//void deleteStringBuf(std::stringbuf* sb);

		template<typename T, typename U>
		void streamCopy(T* fromStream, U* toStream)
		{
			uint32 n = static_cast<uint32>(fromStream->rdbuf()->in_avail());
			char* c = new char[n];
			fromStream->readsome(c, n);
			toStream->write(c, n);
			delete[] c;
		}

		STORMANCER_DLL_API char* readToEnd(bytestream* stream, std::streamsize* length);

#pragma endregion

#pragma region time

		STORMANCER_DLL_API time_t nowTime_t();
		STORMANCER_DLL_API std::string time_tToStr(time_t& time, bool local = false);
		STORMANCER_DLL_API std::string time_tToStr(time_t& time, const char* format);
		STORMANCER_DLL_API std::string nowStr(bool local = false);
		STORMANCER_DLL_API std::string nowStr(const char* format);
		STORMANCER_DLL_API std::string nowDateStr(bool local = false);
		STORMANCER_DLL_API std::string nowTimeStr(bool local = false);
		void writetime(std::ostream &os, std::time_t tc, const char* format); // alternative to std::put_time (not in libstdc++4.9)

#pragma endregion

#pragma region other

		bool ensureSuccessStatusCode(int statusCode);

		template<typename T>
		uint64 ptrToUint64(T* ptr)
		{
			return *static_cast<uint64*>(static_cast<void*>(ptr));
		}

#pragma endregion
};
