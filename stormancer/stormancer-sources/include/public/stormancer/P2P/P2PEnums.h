#pragma once

#include "stormancer/BuildConfig.h"

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
