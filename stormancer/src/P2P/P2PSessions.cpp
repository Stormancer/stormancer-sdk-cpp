#include "stdafx.h"
#include "P2P/P2PSessions.h"
#include "IConnectionManager.h"

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
			_sessions[it->first].Status = sessionState;
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

		_sessions.erase((*it).first);
		auto session = (*it).second;
		auto connection = _connections->getConnection(session.remotePeer);
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

};
