#include "stdafx.h"
#include "Scene.h"
#include "P2P/P2PScenePeer.h"

namespace Stormancer
{
	P2PScenePeer::P2PScenePeer(Scene* scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> p2p)
		: _scene(scene)
		, _connection(connection)
		, _p2p(p2p)
	{
		if (!_connection)
		{
			throw std::runtime_error("Connection cannot be null");
		}
	}

	std::string P2PScenePeer::getSceneId() const
	{
		return _scene->id();
	}

	pplx::task<std::shared_ptr<P2PTunnel>> P2PScenePeer::openP2PTunnel(const std::string& serverId)
	{
		return _p2p->openTunnel((uint64)_connection->id(), _scene->id() + "." + serverId);
	}

	void P2PScenePeer::send(const std::string&, const Writer&, PacketPriority, PacketReliability)
	{
		throw std::runtime_error("Not implemented");
	}

	uint64 P2PScenePeer::id()
	{
		return _connection->id();
	}

	void P2PScenePeer::disconnect()
	{
		throw std::runtime_error("Not implemented");
	}
};
