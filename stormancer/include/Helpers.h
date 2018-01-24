#pragma once

#include "headers.h"
#include "Streams/bytestream.h"

namespace Stormancer
{
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

	template<typename T>
	std::string to_string(const T& t)
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


	/// Join a string vector by using a glue string.
	/// \param vector The vector containing the strings to join.
	/// \param glue A glue string. Default is an empty string.
	std::string vectorJoin(const std::vector<std::string>& vector, const std::string& glue = "");

	std::vector<std::string> stringSplit(const std::string& str, const std::string& separator);

	std::string stringTrim(const std::string& str, char ch = ' ');

	/// Split a string to a vector by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::wstring> wstringSplit(const std::wstring& str, const std::wstring& separator);

	/// Trim a specific character from a string.
	/// \param str The string to trim.
	/// \param ch the character to remove from the string. Default is space.
	std::wstring wstringTrim(const std::wstring& str, wchar_t ch = ' ');

	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

	pplx::task<void> taskDelay(std::chrono::milliseconds timeOffset, pplx::cancellation_token token = pplx::cancellation_token::none());

	template<typename T, typename... U>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>(U...)> func, U&... argument)
	{
		try
		{
			return func(argument...);
		}
		catch (const std::exception& ex)
		{
			return pplx::task_from_exception<T>(ex);
		}
	}

	template<typename T, typename U>
	void streamCopy(T* fromStream, U* toStream)
	{
		uint32 n = static_cast<uint32>(fromStream->rdbuf()->in_avail());
		byte* c = new byte[n];
		fromStream->readsome(c, n);
		toStream->write(c, n);
		delete[] c;
	}

	std::time_t nowTime_t();
	std::string time_tToStr(std::time_t time, bool local = false);
	std::string time_tToStr(std::time_t time, const char* format);
	std::string nowStr(bool local = false);
	std::string nowStr(const char* format);
	std::string nowDateStr(bool local = false);
	std::string nowTimeStr(bool local = false);
	void writetime(std::ostream &os, std::time_t tc, const char* format); // alternative to std::put_time (not in libstdc++4.9)

	bool ensureSuccessStatusCode(int statusCode);

	STORMANCER_DLL_API std::string stringifyBytesArray(const std::vector<byte>& bytes, bool hex = true);

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




	// Cancels the provided task after the specifed delay, if the task
	// did not complete.
	template<typename T>
	pplx::task<T> cancel_after_timeout(pplx::task<T> t, pplx::cancellation_token_source cts, unsigned int timeout)
	{
		// Create a task that returns true after the specified task completes.
		pplx::task<bool> success_task = t.then([](T)
		{
			return true;
		});
		// Create a task that returns false after the specified timeout.
		pplx::task<bool> failure_task = taskDelay(std::chrono::milliseconds(timeout)).then([]
		{
			return false;
		});

		// Create a continuation task that cancels the overall task 
		// if the timeout task finishes first.
		return (failure_task || success_task).then([=](bool success)
		{
			if (!success)
			{
				// Set the cancellation token. The task that is passed as the
				// t parameter should respond to the cancellation and stop
				// as soon as it can.
				cts.cancel();
				throw pplx::task_canceled("task timed out");
			}
			else
			{
				return t.get();
			}

		});
	}

	inline bool is_ipv4_address(const std::string& str)
	{



		struct sockaddr_in sa;
		return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr)) != 0;

		//    struct sockaddr_in sa;
		//    char ip[INET_ADDRSTRLEN];
		//    inet_pton(AF_INET, str.c_str(), &(sa.sin_addr));
		//    inet_ntop(AF_INET, &(sa.sin_addr), ip, INET_ADDRSTRLEN);
		//    return str==string(ip);

		//    char d;
		//    short ip1,ip2,ip3,ip4;
		//    stringstream ss;
		//    ss << str;
		//    ss >> ip1 >> d >> ip2 >> d >> ip3 >> d >> ip4 >> d;
		//    ss.str("");
		//    ss.clear();
		//    ss << (ip1&0xFF) << "." << (ip2&0xFF) << "." << (ip3&0xFF) << "." << (ip4&0xFF);
		//    return ss.str() == str;
	}
	//----------------------------------------------------------------------
	inline bool is_ipv6_address(const std::string& str)
	{



		struct sockaddr_in6 sa;
		return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr)) != 0;


		//    struct sockaddr_in6 sa;
		//    char ip[INET6_ADDRSTRLEN];
		//    inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr));
		//    inet_ntop(AF_INET6, &(sa.sin6_addr), ip, INET6_ADDRSTRLEN);
		//    return str==string(ip);

		//    const char* s=str.c_str();
		//    int colons=0,segsize=0,i,ix=str.length();
		//    if (s[0]==':' || s[ix-1]==':') return false;
		//
		//    for(colons=0,i=0,segsize=0; i<ix; i++)
		//    {
		//        if (s[i]==':')
		//        {
		//            segsize=0;
		//            colons++;
		//        }
		//        else if (('0' <= s[i] && s[i] <='9') || ('a' <= s[i] && s[i] <='f') )//|| ('A' <= s[i] && s[i] <='F')
		//        {
		//            segsize++;
		//        }
		//        else
		//        {
		//            return false;
		//        }
		//        if (segsize>=5) return false;
		//    }
		//    if (colons>7) return false;
		//    return true;
	}


	// The PS Vita doesn't have the codecvt header, which is used by these functions.
	std::wstring utf8_to_wstring(const std::string& str);
	std::string wstring_to_utf8(const std::wstring& str);

};
