#pragma once

#include "stormancer/BuildConfig.h"
namespace Stormancer
{
	enum class MessageOriginFilter
	{
		Host = 1,
		Peer = 2,
		All = 0xFF
	};
}