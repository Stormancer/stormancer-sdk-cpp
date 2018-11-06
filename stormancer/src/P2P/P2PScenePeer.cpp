#include "stormancer/stdafx.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/P2P/P2PScenePeer.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/ChannelUidStore.h"

namespace Stormancer
{
	P2PScenePeer::P2PScenePeer(std::weak_ptr<Scene> scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> p2p, P2PConnectToSceneMessage message)
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
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("Scene destroyed");
		}

		return scene->id();
	}

	pplx::task<std::shared_ptr<P2PTunnel>> P2PScenePeer::openP2PTunnel(const std::string& serverId)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<std::shared_ptr<P2PTunnel>>(std::runtime_error("Unable to establish P2P tunnel: scene destroyed."));
		}
		auto dispatcher = scene->dependencyResolver()->resolve<IActionDispatcher>();		
		return _p2p->openTunnel((uint64)_connection->id(), scene->id() + "." + serverId).then([](auto t) {
			return t.get();
		}, dispatcher);
	}

	void P2PScenePeer::send(const std::string& routeName, const Writer& writer, PacketPriority packetPriority, PacketReliability packetReliability)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("Scene destroyed");
		}
		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string() + "The route '" + routeName + "' doesn't exist on the scene");
		}

		auto route = _remoteRoutesMap[routeName];
		std::stringstream ss;
		ss << "P2PScenePeer_" << scene->id() << "_" << routeName;
		int channelUid = _connection->dependencyResolver()->resolve<ChannelUidStore>()->getChannelUid(ss.str());
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
