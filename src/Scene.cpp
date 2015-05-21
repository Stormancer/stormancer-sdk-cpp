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
			_remoteRoutesMap[routeDto.Name] = Route(this, routeDto.Name, routeDto.Handle, routeDto.Metadata);
		}
	}

	Scene::~Scene()
	{
		disconnect();

		if (_host)
		{
			delete _host;
		}

		for (auto sub : subscriptions)
		{
			sub.unsubscribe();
		}
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

	void Scene::addRoute(wstring routeName, function<void(shared_ptr<Packet<IScenePeer>>)> handler, stringMap metadata)
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

		subscriptions.push_back(onMessage(routeName).subscribe(handler));
	}

	rx::observable<shared_ptr<Packet<IScenePeer>>> Scene::onMessage(Route* route)
	{
		auto observable = rx::observable<>::create<shared_ptr<Packet<IScenePeer>>>([this, route](rx::subscriber<shared_ptr<Packet<IScenePeer>>> subscriber) {
			auto handler = new function<void(shared_ptr<Packet<>>)>([this, subscriber](shared_ptr<Packet<>> p) {
				shared_ptr<Packet<IScenePeer>> packet(new Packet<IScenePeer>(host(), p->stream, p->metadata()));
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

	rx::observable<shared_ptr<Packet<IScenePeer>>> Scene::onMessage(wstring routeName)
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
			throw exception(string(StringFormat(L"The route '{0}' doesn't exist on the scene.", routeName)).c_str());
		}
		Route& route = _remoteRoutesMap[routeName];

		_peer->sendToScene(_handle, route._handle, writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		if (!_connected && !connectCalled)
		{
			connectCalled = true;
			connectTask = _client->connectToScene(this, _token, Helpers::mapValuesPtr(_localRoutesMap));
			connectTask.then([this]() {
				this->_connected = true;
			});
		}
		return connectTask;
	}

	pplx::task<void> Scene::disconnect()
	{
		if (_connected && !disconnectCalled)
		{
			disconnectCalled = true;
			disconnectTask = _client->disconnect(this, _handle);
			this->_connected = false;
		}
		return disconnectTask;
	}

	void Scene::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second._handle = cr.RouteMappings[pair.first];
			_handlers[pair.second._handle] = pair.second.handlers;
		}
	}

	void Scene::handleMessage(shared_ptr<Packet<>> packet)
	{
		packetReceived(packet);

		uint16 routeId;
		*packet->stream >> routeId;

		packet->setMetadata(L"routeId", new uint16(routeId));

		if (Helpers::mapContains(_handlers, routeId))
		{
			auto observers = _handlers[routeId];
			for (size_t i = 0; i < observers.size(); i++)
			{
				(*(observers[i]))(packet);
			}
		}

		delete packet->getMetadata<uint16>(L"routeId");
	}

	vector<IScenePeer*> Scene::remotePeers()
	{
		return vector < IScenePeer* > {host()};
	}

	IScenePeer* Scene::host()
	{
		if (!_host)
		{
			_host = new ScenePeer(_peer, _handle, _remoteRoutesMap, this);
		}
		return _host;
	}
};
