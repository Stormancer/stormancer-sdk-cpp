#pragma once

#include "stormancer/BuildConfig.h"

#include <cstdint>

// custom types
namespace Stormancer
{
#if defined(_STDINT) || defined(__CLANG_STDINT_H)
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using float32 = float;
	using float64 = double;
#else
	using int8 = signed char;
	using int16 = signed short int;
	using int32 = signed long int;
	using int64 = signed long long int;

	using uint8 = unsigned char;
	using uint16 = unsigned short int;
	using uint32 = unsigned long int;
	using uint64 = unsigned long long int;

	using float32 = float;
	using float64 = double;
#endif



	using byte = uint8;

};
