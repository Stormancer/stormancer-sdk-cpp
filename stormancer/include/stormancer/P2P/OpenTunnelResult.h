#pragma once
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	struct OpenTunnelResult
	{
		bool useTunnel;
		std::string endpoint;
		byte handle;

		MSGPACK_DEFINE(useTunnel, endpoint, handle)
	};
}
