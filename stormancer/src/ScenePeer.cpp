#include "stdafx.h"
#include "ScenePeer.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(IConnection* connection, byte sceneHandle, std::map<std::string, Route_ptr>& routeMapping, Scene* scene)
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
		if (!mapContains(_routeMapping, routeName))
		{
			throw std::invalid_argument(std::string("The routeName '") + routeName + "' is not declared on the server.");
		}
		Route_ptr r = _routeMapping[routeName];
		std::stringstream ss;
		ss << "ScenePeer_" << id() << "_" << routeName;
		int channelUid = _connection->getChannelUidStore().getChannelUid(ss.str());
		_connection->send([=, &writer](obytestream* stream) {
			(*stream) << _sceneHandle;
			(*stream) << r->handle();
			writer(stream);
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
		return _connection->id();
	}
};
