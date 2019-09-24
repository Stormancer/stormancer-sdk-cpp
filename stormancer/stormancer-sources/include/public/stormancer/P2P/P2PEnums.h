#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	enum class P2PConnectionStateChangeType
	{
		Connecting,
		Connected,
		Disconnected,
	};

	enum class P2PSessionState
	{
		Connecting,
		Connected,
		Closing,
		Unknown,
	};

	enum class EndpointCandidateType
	{
		Nat,
		Private,
		Public
	};

	enum class P2PTunnelSide
	{
		Host,
		Client
	};
}

MSGPACK_ADD_ENUM(Stormancer::P2PSessionState)
