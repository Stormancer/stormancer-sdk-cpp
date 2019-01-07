#pragma once

#include "stormancer/BuildConfig.h"

#if !defined(_NO_MSGPACK)

#if defined(UE_EDITOR) || defined(UE_GAME)
// msgpack includes windows.h which is problematic with UE
#ifdef _WIN32
#include "AllowWindowsPlatformTypes.h"
#endif // _WIN32
// UE's "check" macro conflicts with msgpack
#define UE_check check
#undef check
#endif // defined(UE_EDITOR) || defined(UE_GAME)

#include "msgpack.hpp"

#if defined(UE_EDITOR) || defined(UE_GAME)

#define check UE_check
#undef UE_check

#ifdef _WIN32
#include "HideWindowsPlatformTypes.h"
#endif // _WIN32

#endif // defined(UE_EDITOR) || defined(UE_GAME)

#else

#define MSGPACK_DEFINE(...)
#define MSGPACK_DEFINE_MAP(...)
#define MSGPACK_DEFINE_ARRAY(...)
#define MSGPACK_ADD_ENUM(...)

#endif
