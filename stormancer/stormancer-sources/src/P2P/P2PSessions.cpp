#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PSessions.h"
#include "stormancer/IConnectionManager.h"

namespace Stormancer
{
	P2PSessions::P2PSessions(std::shared_ptr<IConnectionManager> connections)
		: _connections(connections)
	{
	}

	void P2PSessions::updateSessionState(std::string sessionId, P2PSessionState sessionState)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);

		auto it = _sessions.find(sessionId);
		if (it != _sessions.end())
		{
			it->second.Status = sessionState;
		}
	}

	void P2PSessions::createSession(std::string sessionId, P2PSession session)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);
		auto result = _sessions.insert( {sessionId, session} );
		if (!result.second)
		{
			throw std::runtime_error("Session already created.");
		}
	}

	bool P2PSessions::closeSession(std::string sessionId)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);
		auto it = _sessions.find(sessionId);
		if (it == _sessions.end())
		{
			throw std::runtime_error("Session not found");
		}

		const auto& session = (*it).second;
		auto connection = _connections->getConnection(session.remotePeer);
		_sessions.erase(it);
		if (connection)
		{
			connection->close();
			return true;
		}
		else
		{
			return false;
		}
	}
}
