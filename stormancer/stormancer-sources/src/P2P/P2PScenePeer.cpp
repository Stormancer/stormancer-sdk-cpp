#include "stormancer/stdafx.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/P2P/P2PScenePeer.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/SafeCapture.h"
#include <sstream>

namespace Stormancer
{
	P2PScenePeer::P2PScenePeer(std::weak_ptr<Scene> scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> p2pService, P2PConnectToSceneMessage message)
		: _scene(scene)
		, _connection(connection)
		, _p2pService(p2pService)
		, _handle(message.sceneHandle)
		, _metadata(message.sceneMetadata)
	{
		for (auto r : message.routes)
		{
			_routes.insert({ r.Name, std::make_shared<Route>(r.Name, r.Handle, MessageOriginFilter::Peer, r.Metadata) });
		}

		if (!_connection)
		{
			throw std::runtime_error("Connection cannot be null");
		}
	}

	uint64 P2PScenePeer::id() const
	{
		return _connection->id();
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

	std::shared_ptr<IConnection> P2PScenePeer::connection() const
	{
		return _connection;
	}

	const std::unordered_map<std::string, Route_ptr>& P2PScenePeer::routes() const
	{
		return _routes;
	}

	void P2PScenePeer::send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority packetPriority, PacketReliability packetReliability, const std::string& channelIdentifier)
	{
		auto scene = LockOrThrow(_scene);
		scene->send(routeName, streamWriter, packetPriority, packetReliability, channelIdentifier);
	}

	void P2PScenePeer::disconnect()
	{
		throw std::runtime_error("Not implemented");
	}

	pplx::task<std::shared_ptr<P2PTunnel>> P2PScenePeer::openP2PTunnel(const std::string& serverId, pplx::cancellation_token ct)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<std::shared_ptr<P2PTunnel>>(std::runtime_error("Unable to establish P2P tunnel: scene destroyed."), ct);
		}
		auto dispatcher = scene->dependencyResolver().resolve<IActionDispatcher>();
		pplx::task_options options(dispatcher);
		if (ct.is_cancelable())
		{
			options.set_cancellation_token(ct);
		}

		return _p2pService->openTunnel((uint64)_connection->id(), scene->id() + "." + serverId, ct)
			.then([](pplx::task<std::shared_ptr<P2PTunnel>> t)
		{
			return t.get();
		}, options);
	}
}
