#pragma once

//For static libs
#ifndef _NO_ASYNCRTIMP
#define _NO_ASYNCRTIMP
#endif
#ifndef _NO_PPLXIMP
#define _NO_PPLXIMP
#endif
#define _RAKNET_LIB

//Name      Version  _MSC_VER
//VS 6        6.0      1200
//VS 2002     7.0      1300
//VS 2003     7.1      1310
//VS 2005     8.0      1400
//VS 2008     9.0      1500
//VS 2010    10.0      1600
//VS 2012    11.0      1700
//VS 2013    12.0      1800
//VS 2015    14.0      1900
//VS 2017    15.0      1910

#if !defined(_STORMANCERSDKCPP) 


































#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "iphlpapi.lib") // Needed by libupnp
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Crypt32.lib")

#pragma comment(lib, "Bcrypt.lib")

#endif
// DLL IMPORT / EXPORT
#ifdef _NO_ASYNCRTIMP
#define STORMANCER_DLL_API
#else
#ifdef STORMANCER_DLL_EXPORT
#define STORMANCER_DLL_API __declspec(dllexport) 
#else
#define STORMANCER_DLL_API __declspec(dllimport) 
#endif
#endif


#define CPPREST_FORCE_PPLX 1

//#define STORMANCER_LOG_RPC
//#define STORMANCER_LOG_PACKETS
//#define STORMANCER_LOG_RAKNET_PACKETS
//#define STORMANCER_PACKETFILELOGGER


#if (defined(UE_EDITOR) || defined(UE_GAME)) && defined(_WIN32)
#include "AllowWindowsPlatformTypes.h"
#endif

// standard libs
//#include <mutex>
//#include <condition_variable>
//#include <algorithm>
//#include <chrono>
#include <functional>
//#include <locale>
//#include <iostream>
//#include <map>
#include <memory>
//#include <sstream>
//#include <cstdint>
//#include <cstdio>
#include <string>
//#include <typeinfo>
#include <vector>
//#include <list>
//#include <iomanip>
//#include <thread>
//#include <regex>
//#include <numeric>
//#include <list>
//#include <forward_list>
//#include <stdexcept>

#if defined(_WIN32) //&& defined(_STORMANCERSDKCPP)
// msgpack needs windows.h, RakNet needs WinSock2.h
// WinSock2.h must be included before windows.h
// But msgpack must be included before RakNet on iOS because of a conflict with the 'nil' symbol
// Including WinSock2 directly from here before msgpack and RakNet solves this problem.
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

// cpprestsdk



#ifdef _STORMANCERSDKCPP
#include "cpprest/http_client.h"
#include "cpprest/filestream.h"
	// U is for universal string (char/wchar).
	#ifndef U
	#define U _XPLATSTR
	#endif
#endif



#if defined(UE_EDITOR) || defined(UE_GAME)
#undef check
#endif

#include "stormancer/stormancerTypes.h"

// msgpack
#include "msgpack.hpp"

#if (defined(UE_EDITOR) || defined(UE_GAME))
#include "Runtime/Launch/Resources/Version.h"
#if UE_BUILD_SHIPPING
#define check(expr)                    { CA_ASSUME(expr); }
#else
//Copy paste from UnrealEngine\Engine\Source\Runtime\Core\Public\Misc\AssertionMacros.h
#if ENGINE_MINOR_VERSION < 18
#define check(expr)                { if(UNLIKELY(!(expr))) { FDebug::LogAssertFailedMessage( #expr, __FILE__, __LINE__ ); _DebugBreakAndPromptForRemote(); FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(false); } }
#else
#define check(expr)				{ if(UNLIKELY(!(expr))) { FDebug::LogAssertFailedMessage( #expr, __FILE__, __LINE__, TEXT("") ); _DebugBreakAndPromptForRemote(); FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(false); } }
#endif
#endif // UE_BUILD_SHIPPING
#ifdef _WIN32
#include "HideWindowsPlatformTypes.h"
#endif // _WIN32
#endif // UE_EDITOR || UE_GAME
