#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PSessions.h"
#include "stormancer/IConnectionManager.h"
#include <iostream>
#include <sstream>

namespace Stormancer
{
	P2PSessions::P2PSessions(std::shared_ptr<IConnectionManager> connections)
		: _connections(connections)
	{
	}

	bool P2PSessions::updateSessionState(const std::string& sessionId, P2PSessionState sessionState)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);

		auto it = _sessions.find(sessionId);
		if (it != _sessions.end())
		{
			it->second.Status = sessionState;
			return true;
		}
		return false;
	}

	void P2PSessions::createSession(const std::string& sessionId, const P2PSession& session)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);

		auto result = _sessions.insert( {sessionId, session} );
		if (!result.second)
		{
			throw std::runtime_error("Session already created.");
		}
	}

	bool P2PSessions::closeSession(const std::string& sessionId)
	{
		std::lock_guard<std::mutex> lg(_sessionsMutex);

		auto it = _sessions.find(sessionId);
		if (it == _sessions.end())
		{
			return false;
		}

		const auto& session = (*it).second;
		auto connection = _connections->getConnection(session.RemotePeerId);
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

	P2PSession& P2PSessions::tryGetSession(const std::string& sessionId, bool& found)
	{
		found = false;

		std::lock_guard<std::mutex> lg(_sessionsMutex);

		auto sessionIt = _sessions.find(sessionId);
		if (sessionIt != _sessions.end())
		{
			found = true;
			return sessionIt->second;
		}
		return _dummySession;
	}
}
