#pragma once

// DLL IMPORT / EXPORT
#ifdef STORMANCER_DLL_EXPORT
#define STORMANCER_DLL_API __declspec(dllexport) 
#else
#define STORMANCER_DLL_API __declspec(dllimport) 
#endif

// standart libs
#include <mutex>
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <typeinfo>
#include <vector>
#include <iomanip>
#include <thread>
#include <regex>

// custom libs

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include <pplx/pplxtasks.h>

#include <rx.hpp>

#include <RakPeerInterface.h>
#include <BitStream.h>
#include <PacketPriority.h>
#include <RakNetTypes.h>

// packed libs
#include <msgpack.hpp>

// custom types
#include "typedef.h"
