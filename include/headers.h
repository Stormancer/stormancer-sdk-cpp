#pragma once

// DLL IMPORT / EXPORT
#ifdef STORMANCER_DLL_EXPORT
#define STORMANCER_DLL_API __declspec(dllexport) 
#else
#define STORMANCER_DLL_API __declspec(dllimport) 
#endif

// standart libs
#include <mutex>
#include <atomic>
#include <algorithm>
#include <clocale>
#include <ctime>
#include <cctype>
#include <cstdarg>
#include <codecvt>
#include <functional>
#include <future>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <streambuf>
#include <string>
#include <tchar.h>
#include <typeinfo>
#include <vector>

// custom libs

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include <ppltasks.h>
#include <pplx/pplxtasks.h>

#include <rx.hpp>

namespace rx = rxcpp;
namespace rxu = rxcpp::util;
namespace rxsc = rxcpp::schedulers;
namespace rxsub = rxcpp::subjects;

#include <RakPeerInterface.h>
#include <BitStream.h>
#include <PacketPriority.h>
#include <RakNetTypes.h>

#include <vld.h>

// packed libs
#include <MsgPack.h>

// custom types
#include "typedef.h"
