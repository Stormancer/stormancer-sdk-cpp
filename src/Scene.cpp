#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(shared_ptr<IConnection> connection, Client* client, wstring id, wstring token, SceneInfosDto dto)
		: _id(id),
		_peer(connection),
		_token(token),
		_client(client),
		_metadata(dto.Metadata)
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = Route(this, routeDto.Name, routeDto.Metadata);
		}
	}

	Scene::~Scene()
	{
	}

	wstring Scene::getHostMetadata(wstring key)
	{
		return _metadata[key];
	}

	byte Scene::handle()
	{
		return _handle;
	}

	wstring Scene::id()
	{
		return _id;
	}

	bool Scene::connected()
	{
		return _connected;
	}

	shared_ptr<IConnection> Scene::hostConnection()
	{
		return _peer;
	}

	vector<Route> Scene::localRoutes()
	{
		return Helpers::mapValues(_localRoutesMap);
	}

	vector<Route> Scene::remoteRoutes()
	{
		return Helpers::mapValues(_remoteRoutesMap);
	}

	void Scene::addRoute(wstring routeName, function<void(Packet<IScenePeer>)> handler, stringMap metadata)
	{
		if (routeName.length() == 0 || routeName[0] == '@')
		{
			throw string("A route cannot be empty or start with the '@' character.");
		}

		if (_connected)
		{
			throw string("You cannot register handles once the scene is connected.");
		}

		if (!Helpers::mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route(this, routeName, metadata);
		}

		onMessage(routeName).subscribe(handler);
	}

	rx::observable<Packet<IScenePeer>> Scene::onMessage(Route route)
	{
		auto ob = rx::observable<>::create<Packet<IScenePeer>>([this, route](rx::subscriber<Packet<IScenePeer>> observer) {
			function<void(Packet<>)> action = [this, observer](Packet<> data) {
				Packet<IScenePeer> packet(host(), data.stream, data.metadata());
				observer.on_next(packet);
			};
			route.handlers.push_back(action);

			return [route, action]() {
				auto it = find(route.handlers.begin(), route.handlers.end(), action);
				route.handlers.erase(it);
			};
		});
		return ob;
	}

	rx::observable<Packet<IScenePeer>> Scene::onMessage(wstring routeName)
	{
		if (_connected)
		{
			throw string("You cannot register handles once the scene is connected.");
		}

		if (!Helpers::mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route(this, routeName);
		}

		Route& route = _localRoutesMap[routeName];
		onMessage(route);
	}

	void Scene::sendPacket(wstring routeName, function<void(byteStream&)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (routeName.length() == 0)
		{
			throw string("routeName is empty.");
		}
		if (!_connected)
		{
			throw string("The scene must be connected to perform this operation.");
		}
		if (!Helpers::mapContains(_remoteRoutesMap, routeName))
		{
			throw string("The routeName doesn't exist on the scene.");
		}
		Route& route = _remoteRoutesMap[routeName];

		_peer.get()->sendToScene(_handle, route.index, writer, priority, reliability);
	}

	task<void> Scene::connect()
	{
		return _client->connectToScene(this, _token, _localRoutesMap).then([this]() {
			_connected = true;
		});
	}

	task<void> Scene::disconnect()
	{
		return _client.disconnect(this, _handle);
	}

	void Scene::completeConnectionInitialization(ConnectionResult cr)
	{
		_handle = cr.SceneHandle;

		for (auto& pair : _localRoutesMap)
		{
			pair.second.index = cr.RouteMappings[pair.first];
			_handlers[pair.second.index] = pair.second.handlers;
		}
	}

	void Scene::handleMessage(Packet<> packet)
	{
		auto ev = packetReceived;
		ev(packet);
		byte tmp[2];
		tmp[0] = packet.stream.sbumpc();
		tmp[1] = packet.stream.sbumpc();
		auto routeId = tmp[0] * 256 + tmp[1];

		packet.metadata["routeId"] = routeId;

		if (_handlers.find(routeId) != _handlers.end())
		{
			auto observer = _handlers[routeId];
			observer(packet);
		}
	}

	vector<shared_ptr<IScenePeer>> Scene::remotePeers()
	{
		return vector < shared_ptr<IScenePeer> > { host() };
	}

	shared_ptr<IScenePeer> Scene::host()
	{
		return make_shared<IScenePeer>(new ScenePeer(_peer, _handle, _remoteRoutesMap, this));
	}
};
