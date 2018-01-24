#pragma once

#include "Streams/obytestream.h"
#include "Streams/ibytestream.h"

namespace Stormancer
{
	using Writer = std::function<void(obytestream*)>;

	template<typename TOutput>
	using Unwriter = std::function<TOutput(ibytestream*)>;
};
