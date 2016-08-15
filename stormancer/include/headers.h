#pragma once

// DLL IMPORT / EXPORT
#ifdef STORMANCER_DLL_EXPORT
#define STORMANCER_DLL_API __declspec(dllexport) 
#else
#define STORMANCER_DLL_API __declspec(dllimport) 
#endif

// DEFINES
#define CPPREST_FORCE_PPLX 1
#define STORMANCER_LOG_CLIENT
//#define STORMANCER_LOG_PACKETS
//#define STORMANCER_LOG_RAKNET_PACKETS
//#define STORMANCER_LOG_RPC

#if defined(UE_EDITOR) || defined(UE_GAME)
#include "AllowWindowsPlatformTypes.h"
#endif

// standard libs
#include <mutex>
#include <condition_variable>
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
#include <list>
#include <iomanip>
#include <thread>
#include <regex>
#include <numeric>
#include <list>
#include <forward_list>

// custom libs

// cpprestsdk
#include <../cpprestsdk/Release/include/cpprest/http_client.h>
#include <../cpprestsdk/Release/include/cpprest/filestream.h>
#include <../cpprestsdk/Release/include/pplx/pplxtasks.h>

// rxcpp
#include <../rxcpp/Rx/v2/src/rxcpp/rx.hpp>

// raknet
#include <PacketPriority.h>
#include <RakPeerInterface.h>
#include <BitStream.h>
#include <RakNetTypes.h>
#include <DS_Map.h>
#include <NatPunchthroughClient.h>

// msgpack
#include <msgpack.hpp>

// custom types
#include "typedef.h"

#if defined(UE_EDITOR) || defined(UE_GAME)
#include "HideWindowsPlatformTypes.h"
#endif
