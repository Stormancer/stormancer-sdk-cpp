#pragma once

#include "stormancer/BuildConfig.h"

#if (__cplusplus >= 201703L)
	#define STORM_NODISCARD [[nodiscard]]
#else
	#define STORM_NODISCARD
#endif
