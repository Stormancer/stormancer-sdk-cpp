#include "stormancer/stdafx.h"
#ifdef _WIN32
#include <Ws2tcpip.h>
#endif
#include "stormancer/Helpers.h"
#include <iomanip>
#include <sstream>

namespace Stormancer
{
	bool ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	std::time_t nowTime_t()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	std::string time_tToStr(std::time_t time, bool local)
	{
		return time_tToStr(time, (local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ"));
	}

	std::string time_tToStr(std::time_t time, const char* format)
	{
		std::stringstream ss;
#if defined(_WIN32)
		struct tm timeinfo;
		auto err = localtime_s(&timeinfo, &time);
		if (err == 0)
		{
			ss << std::put_time((tm*)&timeinfo, format);
		}
#else
		//		struct tm timeinfo;
		//		localtime_r(&time, &timeinfo); // POSIX
		//		ss << std::put_time((tm*)&timeinfo, format); // put_time not implemented in libstdc++4.9
		writetime(ss, time, format);
#endif
		return ss.str();
	}

	void writetime(std::ostream &os, std::time_t time, const char* format)
	{




		std::locale loc;
		struct tm timeinfo;
		const std::time_put<char>& tmput = std::use_facet<std::time_put<char>>(loc);
		//std::tm* now = std::localtime(&time);
#if defined(_WIN32)
		localtime_s(&timeinfo, &time);


#else
		localtime_r(&time, &timeinfo); // POSIX
#endif
		tmput.put(os, os, ' ', &timeinfo, format, format + strlen(format));

	}

	std::string nowStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, local);
	}

	std::string nowStr(const char* format)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, format);
	}

	std::string nowDateStr(bool)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%Y-%m-%d");
	}

	std::string nowTimeStr(bool)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, "%H:%M:%S");
	}

	std::string stringifyBytesArray(const byte* data, std::size_t size, bool hex, bool withSpaces)
	{
		std::stringstream ss;
		if (hex)
		{
			ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
		}

		for (std::size_t i = 0; i < size; i++)
		{
			const byte* b = data + i;
			if (withSpaces && i)
			{
				ss << ' ';
			}
			ss << std::setw(2) << (int)(*b);
		}
		return ss.str();
	}

	std::string stringifyBytesArray(const char* data, std::size_t size, bool hex, bool withSpaces)
	{
		return stringifyBytesArray(reinterpret_cast<const byte*>(data), size, hex, withSpaces);
	}

	std::string stringifyBytesArray(const std::vector<byte>& bytes, bool hex, bool withSpaces)
	{
		return stringifyBytesArray(bytes.data(), bytes.size(), hex, withSpaces);
	}

	std::string stringifyBytesArray(const std::vector<char>& bytes, bool hex, bool withSpaces)
	{
		return stringifyBytesArray(bytes.data(), bytes.size(), hex, withSpaces);
	}

	std::string stringifyBytesArray(const std::string& bytes, bool hex, bool withSpaces)
	{
		return stringifyBytesArray(bytes.data(), bytes.size(), hex, withSpaces);
	}

	bool compareExchange(std::function<bool()> const compare, std::function<void()> exchange)
	{
		if (compare())
		{
			exchange();
			return true;
		}
		return false;
	}


	void setUnobservedExceptionHandler(std::function<bool(std::exception_ptr)> handler)
	{
		std::lock_guard<std::mutex> lg(pplx::UnobservedExceptionHandler::unobservedExceptionHandlerMutex);
		pplx::UnobservedExceptionHandler::unobservedExceptionHandler = handler;
	}


	bool is_ipv4_address(const std::string& str)
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

	bool is_ipv6_address(const std::string& str)
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
}
