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

		void updateSessionState(const std::string& sessionId, P2PSessionState sessionState);

		void createSession(const std::string& sessionId, const P2PSession& session);

		bool closeSession(const std::string& sessionId);

		bool tryGetSession(const std::string& p2pSessionId, P2PSession& session);

	private:

		std::shared_ptr<IConnectionManager> _connections;
		std::unordered_map<std::string, P2PSession> _sessions;
		std::mutex _sessionsMutex;
	};
}
