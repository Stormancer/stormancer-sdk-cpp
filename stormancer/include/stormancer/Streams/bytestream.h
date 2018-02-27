#pragma once

#include "stormancer/Streams/obytestream.h"
#include "stormancer/Streams/ibytestream.h"

namespace Stormancer
{
	using Writer = std::function<void(obytestream*)>;

	template<typename TOutput>
	using Unwriter = std::function<TOutput(ibytestream*)>;
};
