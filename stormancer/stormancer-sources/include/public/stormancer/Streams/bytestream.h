#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Streams/obytestream.h"
#include "stormancer/Streams/ibytestream.h"
#include <functional>

namespace Stormancer
{
	using Writer = std::function<void(obytestream*)>;

	template<typename TOutput>
	using Unwriter = std::function<TOutput(ibytestream*)>;
};
