#include "stormancer.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(shared_ptr<IConnection>& connection, byte sceneHandle, map<wstring, Route>& routeMapping, Scene* scene)
		: _connection(connection),
		_sceneHandle(sceneHandle),
		_routeMapping(routeMapping),
		_scene(scene)
	{
	}

	ScenePeer::~ScenePeer()
	{
	}

	void ScenePeer::send(wstring& routeName, function<void(byteStream&)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (!Helpers::mapContains(_routeMapping, routeName))
		{
			throw string(Helpers::StringFormat(L"The routeName '{0}' is not declared on the server.", routeName));
		}
		Route& r = _routeMapping[routeName];
		_connection.get()->sendToScene(_sceneHandle, r.index, writer, priority, reliability);
	}

	void ScenePeer::disconnect()
	{
		_scene->disconnect();
	}

	uint64 ScenePeer::id()
	{
		return _connection.get()->id;
	}
};
