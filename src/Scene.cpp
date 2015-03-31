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

	void Scene::sendPacket(wstring route, function<void(byteStream&)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE)
	{

	}

	task<void> Scene::connect()
	{

	}

	task<void> Scene::disconnect()
	{

	}

	void Scene::completeConnectionInitialization(ConnectionResult cr)
	{

	}
	
	void Scene::handleMessage(Packet<> packet)
	{

	}

	vector<IScenePeer> Scene::remotePeers()
	{

	}

	shared_ptr<IScenePeer> Scene::host()
	{
		return make_shared<IScenePeer>(new ScenePeer(_peer, _handle, _remoteRoutesMap, this));
	}
};
