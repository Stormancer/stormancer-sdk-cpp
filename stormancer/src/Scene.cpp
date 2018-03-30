#include "stormancer/stdafx.h"
#include "stormancer/Scene.h"
#include "stormancer/ScenePeer.h"
#include "stormancer/Client.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, std::weak_ptr<Client> client, const std::string& id, const std::string& token, const SceneInfosDto& dto, std::weak_ptr<DependencyResolver> parentDependencyResolver)
		: _dependencyResolver(std::make_shared<DependencyResolver>(parentDependencyResolver))
		, _peer(connection)
		, _token(token)
		, _metadata(dto.Metadata)
		, _id(id)
		, _client(client)
		, _logger(_dependencyResolver->resolve<ILogger>())
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = std::make_shared<Route>(routeDto.Name, routeDto.Handle, MessageOriginFilter::Host, routeDto.Metadata);
		}

		_connectionStateObservable.get_observable().subscribe([=](ConnectionState state) {
			// On next
			_connectionState = state;
		}, [=](std::exception_ptr exptr) {
			// On error
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "Scene", "Connection state change failed", ex.what());
			}
		});
	}

	Scene::~Scene()
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", "deleting scene...", _id);
#endif
		auto& logger = _logger;
		disconnect(true).then([=](pplx::task<void> t) {
			try
			{
				t.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Trace, "Scene", "Scene disconnection failed.", ex.what());
			}
		}).wait();

		for (auto weakSub : _subscriptions)
		{
			auto sub = weakSub.get_weak().lock();
			if (sub)
			{
				sub->unsubscribe();
			}
		}
		_subscriptions.clear();

		_localRoutesMap.clear();
		_remoteRoutesMap.clear();

#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", "Scene deleted.", _id);
#endif
	}

	void Scene::initialize()
	{
		_host = std::make_shared<ScenePeer>(_peer, _handle, _remoteRoutesMap, this);
	}

	std::string Scene::getHostMetadata(const std::string& key) const
	{
		if (mapContains(_metadata, key))
		{
			return _metadata.at(key);
		}
		return std::string();
	}

	byte Scene::handle() const
	{
		return _handle;
	}

	std::string Scene::id() const
	{
		return _id;
	}

	ConnectionState Scene::getCurrentConnectionState() const
	{
		return _connectionState;
	}

	rxcpp::observable<ConnectionState> Scene::getConnectionStateChangedObservable() const
	{
		return _connectionStateObservable.get_observable();
	}

	IConnection* Scene::hostConnection() const
	{
		return _peer;
	}

	std::vector<Route_ptr> Scene::localRoutes() const
	{
		return mapValues(_localRoutesMap);
	}

	std::vector<Route_ptr> Scene::remoteRoutes() const
	{
		return mapValues(_remoteRoutesMap);
	}

	void Scene::addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		auto observable = onMessage(routeName, filter, metadata);
		auto subscription = observable.subscribe(
			// On next
			handler,
			[=](std::exception_ptr exptr)
		{
			// On error
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "Scene", "Error reading message on route '" + routeName + "'", ex.what());
			}
		});
		_subscriptions.push_back(subscription);
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(const std::string& routeName, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted.");
		}

		if (_connectionState != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not disconnected.");
		}

		if (routeName.size() == 0 || routeName[0] == '@')
		{
			throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
		}

		if (mapContains(_localRoutesMap, routeName))
		{
			throw std::runtime_error("A route already exists for this route name.");
		}

		auto route = std::make_shared<Route>(routeName, (uint16)0, filter, metadata);
		_localRoutesMap[routeName] = route;
		return onMessage(route);
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(Route_ptr route)
	{
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([=](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto handler = std::function<void(Packet_ptr)>([=](Packet_ptr p) {
				std::shared_ptr<IScenePeer> origin = nullptr;
				bool isOriginHost = false;
				if (p->connection->id() == host()->id())
				{
					isOriginHost = true;
					origin = _host;
				}
				else
				{
					origin = _connectedPeers[p->connection->id()];
				}
				if (origin)
				{
					if ((isOriginHost && ((int)route->filter() & (int)MessageOriginFilter::Host)) ||
						((!isOriginHost) && ((int)route->filter() & (int)MessageOriginFilter::Peer))) //Filter packet
					{
						Packetisp_ptr packet(new Packet<IScenePeer>(origin, p->stream, p->metadata));
						subscriber.on_next(packet);
					}
				}
			});
			route->handlers.push_back(handler);
			auto it = route->handlers.end();
			it--;
			subscriber.add([=]() {
				route->handlers.erase(it);
			});
		});

		return observable.as_dynamic();
	}

	void Scene::send(const PeerFilter&, const std::string& routeName, const Writer& writer, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted");
		}

		if (_connectionState != ConnectionState::Connected)
		{
			throw std::runtime_error("The scene is not connected");
		}

		if (routeName.length() == 0)
		{
			throw std::invalid_argument("The Route name is invalid");
		}

		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string() + "The route '" + routeName + "' doesn't exist on the scene");
		}

		Route_ptr route = _remoteRoutesMap[routeName];
		int channelUid = 1;
		if (channelIdentifier.empty())
		{
			std::stringstream ss;
			ss << "Scene_" << _id << "_" << routeName;
			channelUid = _peer->getChannelUidStore().getChannelUid(ss.str());
		}
		else
		{
			channelUid = _peer->getChannelUidStore().getChannelUid(channelIdentifier);
		}
		auto writer2 = [=, &writer](obytestream* stream) {
			(*stream) << _handle;
			(*stream) << route->handle();
			if (writer)
			{
				writer(stream);
			}
		};
		TransformMetadata transformMetadata{ _metadata };
		_peer->send(writer2, channelUid, priority, reliability, transformMetadata);
	}

	void Scene::send(const std::string& routeName, const Writer& writer, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		send(MatchSceneHost(), routeName, writer, priority, reliability, channelIdentifier);
	}

	pplx::task<void> Scene::connect(const pplx::cancellation_token& ct)
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		if (_connectionState == ConnectionState::Disconnected)
		{
			auto client = _client.lock();
			if (client)
			{
				_connectTask = client->connectToScene(_id, _token, mapValues(_localRoutesMap), ct);
			}
			else
			{
				_connectTask = pplx::task_from_exception<void>(std::runtime_error("Client is deleted."));
			}
		}
		else if (_connectionState == ConnectionState::Disconnecting)
		{
			_connectTask = pplx::task_from_exception<void>(std::runtime_error("Scene is disconnecting."));
		}

		return _connectTask;
	}

	pplx::task<void> Scene::disconnect(bool immediate)
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", std::string() + "Scene disconnecting, immediate=" + (immediate ? "true" : "false"));
#endif

		std::lock_guard<std::mutex> lg(_disconnectMutex);

		if (_connectionState == ConnectionState::Connected)
		{
			auto client = _client.lock();
			if (client)
			{
				_disconnectTask = client->disconnect(shared_from_this());
			}
			else
			{
				_logger->log(LogLevel::Warn, "Scene", "Client is invalid.");
				_disconnectTask = pplx::task_from_exception<void>(std::runtime_error("Client is invalid."));
			}
		}
		else if (_connectionState == ConnectionState::Connecting)
		{
			_disconnectTask = pplx::task_from_exception<void>(std::runtime_error("Scene is connecting."));
		}
		else if (_connectionState == ConnectionState::Disconnecting)
		{
			_disconnectTask = pplx::task_from_result();
		}
		else if (_connectionState == ConnectionState::Disconnected)
		{
			_disconnectTask = pplx::task_from_result();
		}

		if (immediate)
		{
			// if immediate, we don't propage the exception
			return _disconnectTask.then([=](pplx::task<void> t) {
				try
				{
					t.wait();
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Warn, "Scene", "Scene disconnection failed.", ex.what());
				}
			});
		}
		else
		{
			return _disconnectTask;
		}
	}

	DependencyResolver* Scene::dependencyResolver() const
	{
		return _dependencyResolver.get();
	}

	void Scene::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second->setHandle(cr.RouteMappings[pair.first]);
			_handlers[pair.second->handle()] = pair.second->handlers;
		}
	}

	void Scene::handleMessage(Packet_ptr packet)
	{
		_onPacketReceived(packet);

		uint16 routeHandle;
		*packet->stream >> routeHandle;
		

		if (mapContains(_handlers, routeHandle))
		{
			Route_ptr route;
			for (auto it = _localRoutesMap.begin(); it != _localRoutesMap.end(); ++it)
			{
				if (it->second->handle() == routeHandle)
				{
					route = it->second;
					break;
				}

			}
			if (packet->connection->id() == _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Host))
			{
				return;//the route doesn't accept messages from the scene host.
			}

			if (packet->connection->id() != _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Peer))
			{
				return;//the route doesn't accept messages from the scene host.
			}

			packet->metadata["routeId"] = route->name();
			auto observers = _handlers[routeHandle];
			for (auto f : observers)
			{
				f(packet);
			}
		}
	}

	std::vector<IScenePeer*> Scene::remotePeers() const
	{
		std::vector<IScenePeer*> container;
		container.push_back(host());
		return container;
	}

	IScenePeer* Scene::host() const
	{
		return _host.get();
	}

	bool Scene::isHost() const
	{
		return _isHost;
	}

	std::unordered_map<uint64, std::shared_ptr<IScenePeer>> Scene::connectedPeers() const
	{
		return _connectedPeers;
	}

	rxcpp::observable<P2PConnectionStateChangedArgs> Scene::p2pConnectionStateChanged() const
	{
		return _p2pConnectionStateChangedObservable.get_observable();
	}

	std::shared_ptr<P2PTunnel> Scene::registerP2PServer(const std::string & p2pServerId)
	{
		auto p2p = dependencyResolver()->resolve<P2PService>();
		return p2p->registerP2PServer(this->id() + "." + p2pServerId);
	}

	pplx::task<std::shared_ptr<P2PScenePeer>> Scene::openP2PConnection(const std::string & p2pToken, const pplx::cancellation_token& ct)
	{
		auto p2p = dependencyResolver()->resolve<P2PService>();
		return p2p->openP2PConnection(p2pToken, ct).then([=](pplx::task < std::shared_ptr<IConnection>> t) {
			return std::make_shared<P2PScenePeer>(this, t.get(), p2p, P2PConnectToSceneMessage());
		}, ct);
		//	auto c = t.get();
		//	P2PConnectToSceneMessage message;
		//	return this->sendSystemRequest<P2PConnectToSceneMessage, P2PConnectToSceneMessage>(c,(byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, message).then([c](P2PConnectToSceneMessage m) {
		//		return std::make_tuple(c, m);
		//	});
		//}).then([=](pplx::task<std::tuple<std::shared_ptr<IConnection>,P2PConnectToSceneMessage>> t) {
		//
		//	auto tuple = t.get();
		//	return std::make_shared<P2PScenePeer>(this,std::get<0>(tuple),p2p,std::get<1>(tuple));
		//});
	}

	Action<Packet_ptr>::TIterator Scene::onPacketReceived(std::function<void(Packet_ptr)> callback)
	{
		return _onPacketReceived.push_back(callback);
	}

	Action<Packet_ptr>& Scene::packetReceivedAction()
	{
		return _onPacketReceived;
	}

	void Scene::setConnectionState(ConnectionState state)
	{
		if (_connectionState != state && !_connectionStateObservableCompleted)
		{
			// on_next and on_completed handlers might delete this. Do not use this at all after calling the handlers.
			_connectionStateObservableCompleted = (state == ConnectionState::Disconnected);
			auto subscriber = _connectionStateObservable.get_subscriber();

			subscriber.on_next(state);
			if (state == ConnectionState::Disconnected)
			{
				subscriber.on_completed();
			}
		}
	}

	pplx::task<std::shared_ptr<P2PScenePeer>> Scene::createScenePeerFromP2PConnection(std::shared_ptr<IConnection> /*connection*/, const pplx::cancellation_token& /*ct*/)
	{
		throw std::runtime_error("Not implemented");
	}
};
