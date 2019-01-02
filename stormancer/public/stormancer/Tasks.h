#pragma once

#include "stormancer/BuildConfig.h"

#if (defined(UE_EDITOR) || defined(UE_GAME)) && defined(_WIN32)
#include "AllowWindowsPlatformTypes.h"
#endif

#ifdef _WIN32
#include "Windows.h"
#endif





#if defined(CPPREST_FORCE_PPLX) && CPPREST_FORCE_PPLX != 1
#error "CPPREST_FORCE_PPLX must be defined to 1 on this platform"
#endif
#define CPPREST_FORCE_PPLX 1
#include "pplx/pplxtasks.h"


#if (defined(UE_EDITOR) || defined(UE_GAME)) && defined(_WIN32)
#include "HideWindowsPlatformTypes.h"
#endif