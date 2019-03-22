#pragma once

#include "stormancer/BuildConfig.h"

#if !defined(_NO_MSGPACK)

#if defined(UE_EDITOR) || defined(UE_GAME)
// msgpack includes windows.h which is problematic with UE
#ifdef _WIN32
#if PLATFORM_XBOXONE
#include "XboxOneAllowPlatformTypes.h" // Include mandatory to compile on Xbox one plateform.
#else
#include "AllowWindowsPlatformTypes.h"
#endif
#endif // _WIN32
// UE's "check" macro conflicts with msgpack
#pragma push_macro("check")
#undef check

#if PLATFORM_LINUX // Ignore Unreal Engine warning as error ([-Werror, -Wundef]).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#endif
#endif // defined(UE_EDITOR) || defined(UE_GAME)

#include "msgpack.hpp"

#if defined(UE_EDITOR) || defined(UE_GAME)

#pragma pop_macro("check")

#ifdef _WIN32
#if PLATFORM_XBOXONE
#include "XboxOneHidePlatformTypes.h"
#else
#include "HideWindowsPlatformTypes.h"
#endif
#endif // _WIN32
#if PLATFORM_LINUX
#pragma clang diagnostic pop
#endif
#endif // defined(UE_EDITOR) || defined(UE_GAME)

#else

#define MSGPACK_DEFINE(...)
#define MSGPACK_DEFINE_MAP(...)
#define MSGPACK_DEFINE_ARRAY(...)
#define MSGPACK_ADD_ENUM(...)

#endif
