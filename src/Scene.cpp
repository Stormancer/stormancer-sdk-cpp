#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, Client* client, wstring id, wstring token, SceneInfosDto dto)
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

	IConnection* Scene::hostConnection()
	{
		return _peer;
	}

	vector<Route*> Scene::localRoutes()
	{
		return Helpers::mapValuesPtr(_localRoutesMap);
	}

	vector<Route*> Scene::remoteRoutes()
	{
		return Helpers::mapValuesPtr(_remoteRoutesMap);
	}

	void Scene::addRoute(wstring routeName, function<void(Packet<IScenePeer>*)> handler, stringMap metadata)
	{
		if (routeName.length() == 0 || routeName[0] == '@')
		{
			throw exception("A route cannot be empty or start with the '@' character.");
		}

		if (_connected)
		{
			throw exception("You cannot register handles once the scene is connected.");
		}

		if (!Helpers::mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route(this, routeName, metadata);
		}

		auto truc = onMessage(routeName).subscribe(handler);
	}

	rx::observable<Packet<IScenePeer>*> Scene::onMessage(Route* route)
	{
		auto observable = rx::observable<>::create<Packet<IScenePeer>*>([this, route](rx::subscriber<Packet<IScenePeer>*> subscriber) {
			auto handler = new function<void(Packet<>*)>([this, &subscriber](Packet<>* p) {
				Packet<IScenePeer>* packet = new Packet<IScenePeer>(host(), p->stream, p->metadata());
				subscriber.on_next(packet);
			});
			route->handlers.push_back(handler);

			subscriber.add([route, handler]() {
				auto it = find(route->handlers.begin(), route->handlers.end(), handler);
				route->handlers.erase(it);
				delete handler;
			});
		});
		return observable.as_dynamic();
	}

	rx::observable<Packet<IScenePeer>*> Scene::onMessage(wstring routeName)
	{
		if (_connected)
		{
			throw exception("You cannot register handles once the scene is connected.");
		}

		if (!Helpers::mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route(this, routeName);
		}

		Route* route = &_localRoutesMap[routeName];
		return onMessage(route);
	}

	void Scene::sendPacket(wstring routeName, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (routeName.length() == 0)
		{
			throw exception("routeName is empty.");
		}
		if (!_connected)
		{
			throw exception("The scene must be connected to perform this operation.");
		}

		if (!Helpers::mapContains(_remoteRoutesMap, routeName))
		{
			throw exception(string(Helpers::StringFormat(L"The route '{0}' doesn't exist on the scene.", routeName)).c_str());
		}
		Route& route = _remoteRoutesMap[routeName];

		_peer->sendToScene(_handle, route.index, writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		return _client->connectToScene(this, _token, Helpers::mapValuesPtr(_localRoutesMap)).then([this]() {
			_connected = true;
		});
	}

	pplx::task<void> Scene::disconnect()
	{
		return _client->disconnect(this, _handle);
	}

	void Scene::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second.index = cr.RouteMappings[pair.first];
			_handlers[pair.second.index] = pair.second.handlers;
		}
	}

	void Scene::handleMessage(Packet<>* packet)
	{
		for (auto fun : packetReceived)
		{
			fun(packet);
		}

		byte tmp[2];
		*packet->stream >> tmp[0];
		*packet->stream >> tmp[1];
		uint16 routeId = tmp[0] * 256 + tmp[1];

		packet->setMetadata(L"routeId", new uint16(routeId));

		if (Helpers::mapContains(_handlers, routeId))
		{
			auto observers = _handlers[routeId];
			for (size_t i = 0; i < observers.size(); i++)
			{
				(*(observers[i]))(packet);
			}
		}
	}

	vector<IScenePeer*> Scene::remotePeers()
	{
		return vector < IScenePeer* > { host() };
	}

	IScenePeer* Scene::host()
	{
		return new ScenePeer(_peer, _handle, _remoteRoutesMap, this);
	}
};
