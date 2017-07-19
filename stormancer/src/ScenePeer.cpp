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

	void ScenePeer::send(std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (!mapContains(_routeMapping, routeName))
		{
			throw std::invalid_argument(std::string("The routeName '") + routeName + "' is not declared on the server.");
		}
		Route_ptr r = _routeMapping[routeName];
		_connection->sendToScene(_sceneHandle, r->handle(), writer, priority, reliability);
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
