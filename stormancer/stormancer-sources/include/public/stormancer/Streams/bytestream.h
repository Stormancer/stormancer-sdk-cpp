#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Streams/obytestream.h"
#include "stormancer/Streams/ibytestream.h"
#include <functional>

namespace Stormancer
{
	using StreamWriter = std::function<void(obytestream&)>;
	using StreamReader = std::function<void(ibytestream&)>;
};
