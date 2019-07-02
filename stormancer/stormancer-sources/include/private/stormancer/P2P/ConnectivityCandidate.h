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
		std::vector<byte> sessionId;
		std::vector<byte> id;
		int ping;
		std::vector<byte> listeningPeerSessionId;
		std::vector<byte> clientPeerSessionId;

		MSGPACK_DEFINE(listeningPeer, clientPeer, listeningEndpointCandidate, clientEndpointCandidate, sessionId, id, ping, listeningPeerSessionId, clientPeerSessionId)
	};
}
