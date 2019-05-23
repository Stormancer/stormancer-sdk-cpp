#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/external/ctti/type_id.hpp"
#include "stormancer/StormancerTypes.h"

namespace Stormancer
{
	template<typename T>
	uint64 getTypeHash()
	{
		return ctti::type_id<T>().hash();
	}
}
