#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/P2P/EndpointCandidate.h"

namespace Stormancer
{
	class ConnectivityCandidate
	{
	public:

		uint64 listeningPeer;
		uint64 clientPeer;

		EndpointCandidate listeningEndpointCandidate;
		EndpointCandidate clientEndpointCandidate;
		std::vector<char> sessionId;
		std::vector<char> id;
		int ping;

		MSGPACK_DEFINE(listeningPeer, clientPeer, listeningEndpointCandidate, clientEndpointCandidate, sessionId, id, ping)
	};
}
