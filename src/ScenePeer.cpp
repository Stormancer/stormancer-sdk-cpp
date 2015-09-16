#include "stormancer.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(IConnection* connection, byte sceneHandle, std::map<std::string, Route>& routeMapping, Scene* scene)
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
		Route& r = _routeMapping[routeName];
		_connection->sendToScene(_sceneHandle, r._handle, writer, priority, reliability);
	}

	void ScenePeer::disconnect()
	{
		_scene->disconnect();
	}

	uint64 ScenePeer::id()
	{
		return _connection->id();
	}
};
