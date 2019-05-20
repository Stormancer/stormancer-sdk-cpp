#pragma once

#include "stormancer/BuildConfig.h"

#include <cstdint>

// custom types
namespace Stormancer
{
	using int8 = std::int8_t;
	using int16 = std::int16_t;
	using int32 = std::int32_t;
	using int64 = std::int64_t;

	using uint8 = std::uint8_t;
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;
	using uint64 = std::uint64_t;

	using float32 = float;
	using float64 = double;

#if defined(_LIBCPP_VERSION) /* _LIBCPP_VERSION Needed for libc++ compatibility ; defined if any libc++ std header is included */
	using byte = char;
#else
	using byte = uint8;
#endif
};
