#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/TimerThread.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include <unordered_map>
#include <map>
#include <ctime>
#include <vector>
#include <sstream>

namespace Stormancer
{

#pragma region MEMORY UTILITIES

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

#pragma endregion

#pragma region CONTAINER UTILITIES

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
	std::vector<T>& operator >> (std::vector<T>& v, T& data)
	{
		data = v.pop_back();
		return v;
	}

	template<typename TKey, typename TValue>
	std::vector<TKey> mapKeys(const std::map<TKey, TValue>& map)
	{
		std::vector<TKey> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->first);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	std::vector<TValue> mapValues(const std::map<TKey, TValue>& map)
	{
		std::vector<TValue> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->second);
		}
		return container;
	}

	/// Returns a boolean indicating if the map contains the key.
	template<typename TKey, typename TValue>
	bool mapContains(const std::map<TKey, TValue>& map, const TKey& key)
	{
		return (map.find(key) != map.end()) ? true : false;
	}

	template<typename TKey, typename TValue>
	bool mapContains(const std::unordered_map<TKey, TValue>& map, const TKey& key)
	{
		return (map.find(key) != map.end()) ? true : false;
	}

#pragma endregion

#pragma region STREAM UTILITIES

	template<typename T, typename U>
	void streamCopy(T* fromStream, U* toStream)
	{
		uint32 n = static_cast<uint32>(fromStream->rdbuf()->in_avail());
		byte* c = new byte[n];
		fromStream->readsome(c, n);
		toStream->write(c, n);
		delete[] c;
	}

#pragma endregion

#pragma region DATE UTILITIES

	std::time_t nowTime_t();
	std::string time_tToStr(std::time_t time, bool local = false);
	std::string time_tToStr(std::time_t time, const char* format);
	std::string nowStr(bool local = false);
	std::string nowStr(const char* format);
	std::string nowDateStr(bool local = false);
	std::string nowTimeStr(bool local = false);
	void writetime(std::ostream &os, std::time_t tc, const char* format); // alternative to std::put_time (not in libstdc++4.9)

#pragma endregion

#pragma region HTTP UTILITIES

	bool ensureSuccessStatusCode(int statusCode);

#pragma endregion

#pragma region BYTE UTILITIES

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

	STORMANCER_DLL_API std::string stringifyBytesArray(const std::vector<byte>& bytes, bool hex = true, bool withSpaces = false);

#pragma endregion

#pragma region COMPARE EXCHANGE

	/// Compares the value of var with expected. If those are equal, replaces var with desired and returns true. Otherwise returns false.
	/// \param var Reference to the value to compare with expected.
	/// \param expected Value expected to be found in var.
	/// \param desired Value to store in var if it is as expected.
	/// \return true if var was successfully changed, false otherwise.
	template<typename T>
	bool compareExchange(T& var, T expected, T desired)
	{
		if (var == expected)
		{
			var = desired;
			return true;
		}
		return false;
	}

	/// Compares the value of var with expected. If those are equal, executes the exchange function and returns true. Otherwise returns false.
	/// \param var Reference to the value to compare with expected.
	/// \param expected Value expected to be found in var.
	/// \param exchange Function to execute if var is as expected.
	/// \return true if the exchange function was called, false otherwise.
	template<typename T>
	bool compareExchange(T& var, T expected, std::function<void()> exchange)
	{
		if (var == expected)
		{
			exchange();
			return true;
		}
		return false;
	}

	/// Calls a compare function. If this function returns true, executes the exchange function and returns true. Otherwise returns false.
	/// \param compare Function to call for comparing.
	/// \param exchange Function to execute if the compare function returned true.
	/// \return true if the exchange function was called, false otherwise.
	bool compareExchange(std::function<bool()> const compare, std::function<void()> exchange);

	/// Compares the value of var with expected. If those are equal, locks the mutex, replaces var with desired and returns true. Otherwise returns false.
	/// \param mutex Mutex to lock before replacing, if var is as expected.
	/// \param var Reference to the value to compare with expected.
	/// \param expected Value expected to be found in var.
	/// \param desired Value to store in var if it is as expected.
	/// \return true if var was successfully changed, false otherwise.
	template<typename T>
	bool compareExchange(std::mutex& mutex, T& var, T expected, T desired)
	{
		if (var == expected)
		{
			std::lock_guard<std::mutex> lg(mutex);
			if (var == expected)
			{
				var = desired;
				return true;
			}
		}
		return false;
	}

	/// Compares the value of var with expected. If those are equal, locks the mutex, executes the exchange function and returns true. Otherwise returns false.
	/// \param mutex Mutex to lock before executing the exchange function, if var is as expected.
	/// \param var Reference to the value to compare with expected.
	/// \param expected Value expected to be found in var.
	/// \param exchange Function to execute if var is as expected.
	/// \return true if the exchange function was called, false otherwise.
	template<typename T>
	bool compareExchange(std::mutex& mutex, T& var, T expected, std::function<void()> exchange)
	{
		if (var == expected)
		{
			std::lock_guard<std::mutex> lg(mutex);
			if (var == expected)
			{
				exchange();
				return true;
			}
		}
		return false;
	}

	/// Calls a compare function. If this function returns true, locks the mutex, executes the exchange function and returns true. Otherwise returns false.
	/// \param mutex Mutex to lock before executing the exchange function, if the first compare function call returned true.
	/// \param compare Function to call for comparing. If the first call to compare returns true, compare will be called another time after the mutex lock.
	/// \param exchange Function to execute if the compare function returned true.
	/// \return true if the exchange function was called, false otherwise.
	bool compareExchange(std::mutex& mutex, std::function<bool()> const compare, std::function<void()> exchange);

#pragma endregion

#pragma region NETWORK UTILITIES

	bool is_ipv4_address(const std::string& str);

	bool is_ipv6_address(const std::string& str);

#pragma endregion

#pragma region EXCEPTION UTILITIES


	STORMANCER_DLL_API void setUnobservedExceptionHandler(std::function<bool(std::exception_ptr)> handler);


#pragma endregion

}
