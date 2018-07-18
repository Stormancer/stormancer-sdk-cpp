#include "stormancer/stdafx.h"
#include "stormancer/Scene.h"
#include "stormancer/P2P/P2PScenePeer.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"

namespace Stormancer
{
	P2PScenePeer::P2PScenePeer(Scene* scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> p2p, P2PConnectToSceneMessage message)
		: _scene(scene)
		, _connection(connection)
		, _p2p(p2p)
		, _handle(message.sceneHandle)
		,_metadata(message.metadata)
		
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

	void P2PScenePeer::send(const std::string& routeName, const Writer& writer, PacketPriority packetPriority, PacketReliability packetReliability)
	{
		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string() + "The route '" + routeName + "' doesn't exist on the scene");
		}

		auto route = _remoteRoutesMap[routeName];
		std::stringstream ss;
		ss << "P2PScenePeer_" << _scene->id() << "_" << routeName;
		int channelUid = _connection->getChannelUidStore().getChannelUid(ss.str());
		_connection->send([=, &writer](obytestream* stream) {
			(*stream) << _handle;
			// TODO: (*stream) << routeHandle;
			writer(stream);
		}, channelUid, packetPriority, packetReliability);
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
