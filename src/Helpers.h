#pragma once
#include "headers.h"
#include "basic_bytebuf.h"
#include "basic_bytestream.h"

namespace Stormancer
{
	//template<typename T>
	//class SWrapper
	//{
	//public:
	//	T* data;

	//private:
	//	uint64* refsCount;
	//	std::mutex* _mutex;

	//public:
	//	SWrapper(T* data = nullptr)
	//		: data(data),
	//		refsCount(new uint64(1)),
	//		_mutex(new std::mutex())
	//	{
	//	}

	//	SWrapper(const SWrapper& other)
	//		_mutex(other._mutex)
	//	{
	//		_mutex->lock();
	//		data = other.data;
	//		refsCount = other.refsCount;
	//		++(*refsCount);
	//		_mutex->unlock();
	//	}

	//	SWrapper& operator=(const SWrapper& other)
	//	{
	//		_mutex = other._mutex;
	//		_mutex->lock();
	//		data = other.data;
	//		refsCount = other.refsCount;
	//		_mutex->unlock();
	//	}

	//private:
	//	~SWrapper()
	//	{
	//		if (*refsCount == 0 && data)
	//		{
	//			delete data;
	//		}
	//	}

	//public:
	//	void destroy()
	//	{
	//		delete this;
	//	}
	//};

	template<typename T>
	struct deleter
	{
		void operator()(const T* p)
		{
			delete p;
		}
	};

	template<typename T>
	struct array_deleter
	{
		void operator()(const T* p)
		{
			delete[] p;
		}
	};

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
	T* reverseByteOrder(T* data, uint64 n = 0)
	{
		char* tmp = (char*)data;
		if (n == 0)
		{
			n = sizeof(T);
		}
		std::reverse(tmp, tmp + n);
		return data;
	}

#pragma endregion

#pragma region map

	anyMap stringMapToAnyMap(stringMap& sm);

	template<typename TKey, typename TValue>
	std::vector<TKey> mapKeys(std::map<TKey, TValue>& map)
	{
		std::vector<TKey> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->first);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	std::vector<TValue> mapValues(std::map<TKey, TValue>& map)
	{
		std::vector<TValue> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->second);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TKey> RakMapKeys(DataStructures::Map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		uint32 sz = map.Size();
		for (uint32 i = 0; i < sz; ++i)
		{
			container.Insert(map.GetKeyAtIndex(i), __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TValue> RakMapValues(DataStructures::Map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		uint32 sz = map.Size();
		for (uint32 i = 0; i < sz; ++i)
		{
			container.Insert(map[i], __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TKey> mapToRakListKeys(std::map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.Insert(it->first, __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TValue> mapToRakListValues(std::map<TKey, TValue>& map)
	{
		DataStructures::List<TValue> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.Insert(it->second, __FILE__, __LINE__);
		}
		return container;
	}

	/// Returns a boolean indicating if the map contains the key.
	template<typename TKey, typename TValue>
	bool mapContains(const std::map<TKey, TValue>& map, const TKey& key)
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
	pplx::task<T> taskCompleted(T& result)
	{
		pplx::task_completion_event<T> tce;
		tce.set(result);
		return pplx::create_task(tce);
	}

	template<typename T>
	pplx::task<T> taskFromException(const std::exception& e)
	{
		pplx::task_completion_event<T> tce;
		tce.set_exception<std::exception>(e);
		return create_task(tce);
	}

	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

	template<typename T>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>()> func)
	{
		try
		{
			return func();
		}
		catch (const std::exception& e)
		{
			return taskFromException<T>(e);
		}
	}

	template<typename T, typename U>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>(U)> func, U& argument)
	{
		try
		{
			return func(argument);
		}
		catch (const std::exception& e)
		{
			return taskFromException<T>(e);
		}
	}

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

	std::string stringifyBytesArray(std::string bytes, bool hex = false);

#pragma endregion
};
