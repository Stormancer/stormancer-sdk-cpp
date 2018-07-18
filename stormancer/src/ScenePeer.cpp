#include "stormancer/stdafx.h"
#include "stormancer/ScenePeer.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(std::weak_ptr<IConnection> connection, byte sceneHandle, std::map<std::string, Route_ptr>& routeMapping, Scene* scene)
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
		int channelUid = connection->getChannelUidStore().getChannelUid(ss.str());
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
		if (_scene)
		{
			_scene->disconnect();
		}
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
