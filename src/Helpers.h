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
		std::reverse(tmp, tmp + (n >= 0 ? n : sizeof(T)));
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
		std::vector<TKey*> mapKeysPtr(std::map<TKey, TValue>& map)
		{
			std::vector<TKey*> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(&it->first);
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

		template<typename TKey, typename TValue>
		std::vector<TValue*> mapValuesPtr(std::map<TKey, TValue>& map)
		{
			std::vector<TValue*> vec;
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(&it->second);
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
		pplx::task<T> taskFromException(std::exception& ex)
		{
			pplx::task_completion_event<T> tce;
			tce.set_exception(ex);
			return create_task(tce);
		}

		pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

#pragma endregion

#pragma region stream

		bytestream* convertRakNetPacketToStream(RakNet::Packet* packet);

		void deleteStringBuf(std::stringbuf* sb);

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
		std::string STORMANCER_DLL_API time_tToStr(time_t& time, bool local = false);
		std::string STORMANCER_DLL_API time_tToStr(time_t& time, const char* format);
		std::string STORMANCER_DLL_API nowStr(bool local = false);
		std::string STORMANCER_DLL_API nowStr(const char* format);
		std::string STORMANCER_DLL_API nowDateStr(bool local = false);
		std::string STORMANCER_DLL_API nowTimeStr(bool local = false);
		void writetime(std::ostream &os, std::time_t tc, const char* format); // alternative to std::put_time (not in libstdc++4.9)

#pragma endregion

#pragma region other

		bool ensureSuccessStatusCode(int statusCode);

		template<typename T>
		uint64 ptrToUint64(T* ptr)
		{
			return *static_cast<uint64*>(static_cast<void*>(ptr));
		}

		template<typename P1 = const char*, typename P2 = const char*, typename P3 = const char*, typename P4 = const char*, typename P5 = const char*, typename P6 = const char*, typename P7 = const char*, typename P8 = const char*, typename P9 = const char*, typename P10 = const char*>
		std::string stringFormat(P1 p1 = "", P2 p2 = "", P3 p3 = "", P4 p4 = "", P5 p5 = "", P6 p6 = "", P7 p7 = "", P8 p8 = "", P9 p9 = "", P10 p10 = "")
		{
			std::stringstream ss;
			ss << p1 << p2 << p3 << p4 << p5 << p6 << p7 << p8 << p9 << p10;
			return ss.str();
		}

#pragma endregion
};

template<typename T>
Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, T& data)
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

/// Write a c-string in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The c-string to write.
///	\return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, const char* data);

/// Write a constant std::string in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The std::string to write.
/// \return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, const std::string& data);

/// Write a c-string in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The c-string to write.
/// \return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, std::string& data);

/// Write a wide c-string in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The wide c-string to write.
/// \return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, const wchar_t* data);

/// Write a constant std::wstring in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The std::wstring to write.
/// \return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, const std::wstring& data);

/// Write a std::wstring in a byte stream.
/// \param bs The byte stream we want to write in.
/// \param data The std::wstring to write.
/// \return The byte stream.
STORMANCER_DLL_API Stormancer::bytestream& operator<<(Stormancer::bytestream& bs, std::wstring& data);
#include "ILogger.h"
/// Template for reading any data type from the byte stream.
/// \param bs The byte stream we want to read.
/// \param data A ref to a variable for puting the read data.
template<typename T>
Stormancer::bytestream& operator>>(Stormancer::bytestream& bs, T& data)
{
	Stormancer::ILogger::instance()->log(Stormancer::stringFormat("#### Read ", sizeof(T)));
	Stormancer::ILogger::instance()->log(Stormancer::stringFormat("#1"));
	char* tmp = (char*)&data;
	Stormancer::ILogger::instance()->log(Stormancer::stringFormat("#2"));
	bs.read(tmp, sizeof(T));
	Stormancer::ILogger::instance()->log(Stormancer::stringFormat("#3"));
	//std::memcpy(tmp, data, sizeof(T)):
#ifdef _IS_BIG_ENDIAN
	reverseByteOrder(&data);
#endif
	Stormancer::ILogger::instance()->log(Stormancer::stringFormat("#4"));
	return bs;
}

/// Read a std::string from a byte stream.
/// \param bs The byte stream we want to read.
/// \param data A ref to a std::string where we get the data.
STORMANCER_DLL_API Stormancer::bytestream& operator>>(Stormancer::bytestream& bs, std::string& data);

/// Read a std::wstring from a byte stream.
/// \param bs The byte stream we want to read.
/// \param data A ref to a std::wstring where we get the data.
STORMANCER_DLL_API Stormancer::bytestream& operator>>(Stormancer::bytestream& bs, std::wstring& data);
