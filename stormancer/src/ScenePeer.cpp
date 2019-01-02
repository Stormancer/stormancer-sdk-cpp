#include "stormancer/stdafx.h"
#include "stormancer/ScenePeer.h"
#include "stormancer/ChannelUidStore.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(std::weak_ptr<IConnection> connection, byte sceneHandle, std::map<std::string, Route_ptr>& routeMapping, std::weak_ptr<Scene> scene)
		: _connection(connection),
		_sceneHandle(sceneHandle),
		_routeMapping(routeMapping),
		_scene(scene)
	{
	}

	ScenePeer::~ScenePeer()
	{
	}

	void ScenePeer::send(const std::string& routeName, const Writer& writer, PacketPriority priority, PacketReliability reliability)
	{
		auto connection = _connection.lock();
		if (!connection)
		{
			throw std::runtime_error("Connection deleted.");
		}

		if (!mapContains(_routeMapping, routeName))
		{
			throw std::invalid_argument(std::string("The routeName '") + routeName + "' is not declared on the server.");
		}
		Route_ptr r = _routeMapping[routeName];
		std::stringstream ss;
		ss << "ScenePeer_" << id() << "_" << routeName;
		int channelUid = connection->dependencyResolver()->resolve<ChannelUidStore>()->getChannelUid(ss.str());
		connection->send([=, &writer](obytestream* stream) {
			(*stream) << _sceneHandle;
			(*stream) << r->handle();
			if (writer)
			{
				writer(stream);
			}
		}, channelUid, priority, reliability);
	}

	void ScenePeer::disconnect()
	{
		auto scene = _scene.lock();
		if (scene)
		{
			scene->disconnect();
		}
	}
	std::string ScenePeer::getSceneId() const
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("Scene destroyed");

		}
		return scene->id();
	}
	uint64 ScenePeer::id()
	{
		auto connection = _connection.lock();
		if (!connection)
		{
			throw std::runtime_error("Connection deleted.");
		}

		return connection->id();
	}
};
