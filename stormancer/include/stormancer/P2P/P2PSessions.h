#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/P2P/P2PEnums.h"
#include "stormancer/P2P/P2PSession.h"
#include "stormancer/IConnectionManager.h"

namespace Stormancer
{
	class P2PSessions
	{
	public:

		P2PSessions(std::shared_ptr<IConnectionManager> connections);

		void updateSessionState(std::string sessionId, P2PSessionState sessionState);

		void createSession(std::string sessionId, P2PSession session);

		bool closeSession(std::string sessionId);

	private:

		std::shared_ptr<IConnectionManager> _connections;
		std::unordered_map<std::string, P2PSession> _sessions;
		std::mutex _sessionsMutex;
	};
}
