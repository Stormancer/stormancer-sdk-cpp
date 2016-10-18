#include "stormancer.h"

namespace Stormancer
{
	// ConnectionHandler

	Client::ConnectionHandler::ConnectionHandler()
	{
		IConnectionManager::connectionCount = 0;
	}

	Client::ConnectionHandler::~ConnectionHandler()
	{
	}

	uint64 Client::ConnectionHandler::generateNewConnectionId()
	{
		static uint64 _current = 0;
		return _current++;
	}

	void Client::ConnectionHandler::newConnection(IConnection* connection)
	{
	}

	IConnection* Client::ConnectionHandler::getConnection(uint64 id)
	{
		throw std::runtime_error("Client::ConnectionHandler::getConnection not implemented.");
	}

	void Client::ConnectionHandler::closeConnection(IConnection* connection, std::string reason)
	{
	}


	// Client

	Client::Client(std::shared_ptr<Configuration> config)
		: _initialized(false)
		, _accountId(config->account)
		, _config(config)
		, _applicationName(config->application)
		, _metadata(config->_metadata)
		, _maxPeers(config->maxPeers)
		, _dependencyResolver(new DependencyResolver())
		, _plugins(config->plugins())
		, _synchronisedClock(config->synchronisedClock)
	{
		dependencyResolver()->registerDependency<ITokenHandler>([](DependencyResolver* resolver) {return std::make_shared<TokenHandler>(); });
		dependencyResolver()->registerDependency<ApiClient>([config](DependencyResolver* resolver) {
			auto tokenHandler = resolver->resolve<ITokenHandler>();
			return std::make_shared<ApiClient>(config,tokenHandler);
		},true);
		dependencyResolver()->registerDependency<ILogger>(ILogger::instance());
		dependencyResolver()->registerDependency<IScheduler>(config->scheduler);
		dependencyResolver()->registerDependency(config->transportFactory, true);
		dependencyResolver()->registerDependency<IActionDispatcher>(config->actionDispatcher);
		dependencyResolver()->registerDependency<SceneDispatcher>([](DependencyResolver* resolver) {
			return std::make_shared<SceneDispatcher>(resolver->resolve<IActionDispatcher>());
		}, true);
		dependencyResolver()->registerDependency<SyncClock>([config](DependencyResolver* resolver) {
			return std::make_shared<SyncClock>(resolver, config->synchronisedClockInterval);
		}, true);
		dependencyResolver()->registerDependency<RequestProcessor>([](DependencyResolver* resolver) {
			return std::make_shared<RequestProcessor>(resolver->resolve<ILogger>(), std::vector<IRequestModule*>());
		}, true);

		dependencyResolver()->registerDependency<IPacketDispatcher>([this](DependencyResolver* resolver) {
			auto dispatcher = _config->dispatcher(resolver);
			dispatcher->addProcessor(resolver->resolve<RequestProcessor>());
			dispatcher->addProcessor(resolver->resolve<SceneDispatcher>());
			return dispatcher;
		}, true);

		_connectionStateObservable;

		_logger = _dependencyResolver->resolve<ILogger>();





		_metadata["serializers"] = "msgpack/array";
		_metadata["transport"] = _dependencyResolver->resolve<ITransport>()->name();
		_metadata["version"] = "1.2.0";
		_metadata["platform"] = __PLATFORM__;
		_metadata["protocol"] = "2";

		_connectionStateObservable.get_observable().subscribe([this](ConnectionState state) {
			this->_connectionState = state;
		});

		for (auto plugin : _plugins)
		{
			plugin->clientCreated(this);
		}

		initialize();
	}

	
	Client::~Client()
	{

		ILogger::instance()->log(LogLevel::Trace, "Client::~Client", "deleting the client...", "");


		dependencyResolver()->resolve<IActionDispatcher>()->stop();
		while (dependencyResolver()->resolve<IActionDispatcher>()->isRunning()) //Wait for dispatcher to stop.
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		for (auto plugin : _plugins)
		{
			plugin->destroy();
		}
		_plugins.clear();



		

		if (_serverConnection)
		{
			_serverConnection = nullptr;
		}

		delete _dependencyResolver;

		ILogger::instance()->log(LogLevel::Trace, "Client::~Client", "client deleted", "");

	}

	Client* Client::createClient(std::shared_ptr<Configuration> config)
	{
		if (!config)
		{
			ILogger::instance()->log(LogLevel::Error, "Client::createClient", "Provided client configuration is null", "");
		}

		return new Client(config);
	}

	pplx::task<void> Client::destroy(Client * client)
	{
		return client->disconnect(true).then([client]() {
			delete client;
		});
		
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			_initialized = true;
			dependencyResolver()->resolve<IActionDispatcher>()->start();
			auto transport = _dependencyResolver->resolve<ITransport>();
			transport->onPacketReceived([this](Packet_ptr packet) { transport_packetReceived(packet); });
			transport->onTransportEvent([this](void* data, void* data2, void* data3) {
				for (auto plugin : _plugins)
				{
					plugin->transportEvent(data, data2, data3);
				}
			});

		}
	}

	std::string Client::applicationName()
	{
		return _applicationName;
	}

	std::shared_ptr<ILogger> Client::logger()
	{
		return this->dependencyResolver()->resolve<ILogger>();
	}


	pplx::task<std::shared_ptr<Scene>> Client::getPublicScene(const char* sceneId)
	{
		if (!sceneId)
		{
#ifdef STORMANCER_LOG_CLIENT
			ILogger::instance()->log(LogLevel::Error, "Client::getPublicScene", "Bad scene id", "");
#endif
		}

#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Client::getPublicScene", sceneId, "");
#endif

		return dependencyResolver()->resolve<ApiClient>()->getSceneEndpoint(_accountId, _applicationName, sceneId).then([this, sceneId](pplx::task<SceneEndpoint> t) {

			try
			{
				SceneEndpoint sep = t.get();
				return this->getScene(sceneId, sep);
			}
			catch (const std::exception& ex)
			{
				logger()->log(ex);

				throw std::runtime_error((std::string("An error occured while obtaining the scene endpoints for public scene '") + sceneId + " :" + ex.what()).c_str());
			}
		});
	}

	pplx::task<std::shared_ptr<Scene>> Client::getScene(const char* token2)
	{
		std::string token = token2;
		auto sep = dependencyResolver()->resolve<TokenHandler>()->decodeToken(token);
		return getScene(sep.tokenData.SceneId, sep);
	}

	pplx::task<void> Client::tryConnectToServer(SceneEndpoint endpoint)
	{

		if (this->GetCurrentConnectionState() == ConnectionState::Disconnected)
		{
			std::lock_guard<std::mutex> lock(_connectionMutex);
			if (this->GetCurrentConnectionState() == ConnectionState::Disconnected)
			{
				_connectionState = ConnectionState::Connecting;
				setConnectionState(ConnectionState::Connecting);

				assert(_serverConnection == nullptr);
				try
				{
					auto transport = dependencyResolver()->resolve<ITransport>();

					_logger->log(LogLevel::Trace, "Client::getScene", "Starting transport", ("port :" + std::to_string(this->_config->serverPort) + ", maxPeers:" + std::to_string((uint16)_maxPeers + 1)).c_str());
					//Start transport and execute plugin event
					transport->start("client", new ConnectionHandler(), _cts.get_token(), (uint16)_maxPeers + 1, this->_config->serverPort);
					for (auto plugin : _plugins)
					{
						plugin->transportStarted(transport.get());
					}

					//Connect to server
					std::string endpointUrl = endpoint.tokenData.Endpoints.at(transport->name());

					_logger->log(LogLevel::Trace, "Client::getScene", "Connecting transport to server.", endpointUrl.c_str());
					return transport->connect(endpointUrl)
						.then([this](pplx::task<IConnection*> t)
					{
						auto connection = t.get();

						connection->GetConnectionStateChangedObservable().subscribe([this](ConnectionState state) {

							if (state == ConnectionState::Disconnected)
							{
								auto scenes = _scenes;
								for (auto sceneIt : scenes)
								{
									auto scene = sceneIt.second;
									for (auto plugin : _plugins)
									{
										plugin->sceneDisconnected(scene.get());
									}
									this->dispatchEvent([scene]() {scene->setConnectionState(ConnectionState::Disconnected); });

								}
								setConnectionState(state);

								_serverConnection = nullptr;
							}
						});
						connection->setMetadata(_metadata);
						this->_serverConnection = connection;
						return;
					})
						.then([this](pplx::task<void>)
					{
						return this->updateServerMetadata();
					})
						.then([this, endpoint](pplx::task<void> t)
					{

						try
						{
							if (_synchronisedClock && endpoint.tokenData.Version > 0)
							{
								dependencyResolver()->resolve<SyncClock>()->Start(_serverConnection);
							}
						}
						catch (std::exception& ex)
						{
							throw ex;
						}

					});
				}
				catch (std::exception& ex)
				{

					return pplx::task_from_exception<void, std::runtime_error>(std::runtime_error(std::string(ex.what()) + ": Failed to start transport."));

				}
			}

		}

		return _connectionTask;
	}

	pplx::task<std::shared_ptr<Scene>> Client::getScene(std::string sceneId, SceneEndpoint sep)
	{
		_logger->log(LogLevel::Trace, "Client::getScene", sceneId.c_str(), sep.token.c_str());



		return this->tryConnectToServer(sep).then([this, sep]() {
			SceneInfosRequestDto parameter;
			parameter.Metadata = _serverConnection->metadata();
			parameter.Token = sep.token;

			_logger->log(LogLevel::Trace, "Client::getScene", "send SceneInfosRequestDto", "");

			return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter);
		}).then([this, sep, sceneId](SceneInfosDto sceneInfos) {
			std::stringstream ss;
			ss << sceneInfos.SceneId << " " << sceneInfos.SelectedSerializer << " Routes:[";
			for (uint32 i = 0; i < sceneInfos.Routes.size(); i++)
			{
				ss << sceneInfos.Routes.at(i).Handle << ":" << sceneInfos.Routes.at(i).Name << ";";
			}
			ss << "] Metadata:[";
			for (auto it : sceneInfos.Metadata)
			{
				ss << it.first << ":" << it.second << ";";
			}
			ss << "]";

			_logger->log(LogLevel::Trace, "Client::getScene", "SceneInfosDto received", ss.str().c_str());

			_serverConnection->metadata()["serializer"] = sceneInfos.SelectedSerializer;

			return updateServerMetadata().then([sceneInfos]() {
				return sceneInfos;
			});
		}).then([this, sep, sceneId](pplx::task<SceneInfosDto> t) {

			auto sceneInfos = t.get();
			_logger->log(LogLevel::Trace, "Client::getScene", "Returning the scene ", sceneId.c_str());

			ScenePtr scene;
			if (mapContains(_scenes, sceneId))
			{
				scene = _scenes[sceneId];
			}
			else
			{
				auto sceneDeleter = [this, sceneId]() {
					if (mapContains(_scenes, sceneId))
					{
						_scenes.erase(sceneId);
					}
				};
				scene = std::make_shared<Scene>(_serverConnection, this, sceneId, sep.token, sceneInfos, _dependencyResolver, sceneDeleter);
				_scenes[sceneId] = scene;
			}
			for (auto plugin : _plugins)
			{

				plugin->registerSceneDependencies(scene.get());
			}
			for (auto plugin : _plugins)
			{

				plugin->sceneCreated(scene.get());
			}
			return scene;
		});
	}

	pplx::task<void> Client::updateServerMetadata()
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Client::updateServerMetadata", "sending system request.", "");
#endif
		return dependencyResolver()->resolve<RequestProcessor>()->sendSystemRequest(_serverConnection, (byte)SystemRequestIDTypes::ID_SET_METADATA, [this](bytestream* bs) {
			msgpack::pack(bs, _serverConnection->metadata());
		}).then([](pplx::task<Packet_ptr> t) {
			try
			{
				t.wait();
			}
			catch (std::exception& e)
			{
				throw std::logic_error(std::string(e.what()) + "\nFailed to update the server metadata.");
			}
		});
	}

	pplx::task<void> Client::connectToScene(std::shared_ptr<Scene> scene, std::string& token, std::vector<Route_ptr> localRoutes)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		if (scene->GetCurrentConnectionState() != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not in disconnected state");
		}

		for (auto plugin : _plugins)
		{
			plugin->sceneConnecting(scene.get());
		}

		scene->setConnectionState(ConnectionState::Connecting);

		ConnectToSceneMsg parameter;
		parameter.Token = token;
		for (auto r : localRoutes)
		{
			RouteDto routeDto;
			routeDto.Handle = r->handle();
			routeDto.Metadata = r->metadata();
			routeDto.Name = r->name();
			parameter.Routes << routeDto;
		}
		parameter.ConnectionMetadata = _serverConnection->metadata();

		return Client::sendSystemRequest<ConnectToSceneMsg, ConnectionResult>((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter).then([this, scene](ConnectionResult result) {
			scene->completeConnectionInitialization(result);
			dependencyResolver()->resolve<SceneDispatcher>()->addScene(scene);
			for (auto plugin : _plugins)
			{
				plugin->sceneConnected(scene.get());
			}
			scene->setConnectionState(ConnectionState::Connected);
#ifdef STORMANCER_LOG_CLIENT
			logger()->log(LogLevel::Info, "client", "Scene connected", scene->id().c_str());
#endif
		});
	}

	pplx::task<void> Client::disconnect(bool immediate)
	{
		pplx::task_completion_event<void> tce;


		try
		{
			dependencyResolver()->resolve<SyncClock>()->Stop();

			if (_serverConnection && _serverConnection->connectionState() == ConnectionState::Connected)
			{
				// Stops the synchronised clock, disconnect the scenes, closes the server connection then stops the underlying transports.

				auto dt = disconnectAllScenes(immediate);

				auto end = [this, tce]() {
					_cts.cancel(); // this will cause the transport to stop




					if (_serverConnection)
					{
						_serverConnection->close();
					}
					tce.set();
				};

				if (!immediate)
				{
					dt.then([tce, end](pplx::task<void> t) {
						try
						{
							t.wait();
							end();
						}
						catch (const std::exception& ex)
						{

							tce.set_exception(ex);
						}
					});
				}
				else
				{
					end();
				}
			}
			else
			{

				tce.set();
			}
		}
		catch (const std::exception& ex)
		{

			tce.set_exception(ex);
		}

		return pplx::create_task(tce);
	}

	pplx::task<void> Client::disconnect(Scene* scene, byte sceneHandle, bool immediate)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is null");
		}

		if (scene->GetCurrentConnectionState() != ConnectionState::Connected)
		{
			throw std::runtime_error("The scene is not in connected state");
		}

		scene->setConnectionState(ConnectionState::Disconnecting);

		for (auto plugin : _plugins)
		{
			plugin->sceneDisconnecting(scene);
		}

		auto taskDisconnected = [this, scene]() {
			for (auto plugin : _plugins)
			{
				plugin->sceneDisconnected(scene);
			}

#ifdef STORMANCER_LOG_CLIENT
			std::string sceneId = scene->id();
			ILogger::instance()->log(LogLevel::Info, "client", "Scene disconnected", sceneId.c_str());
#endif

			scene->setConnectionState(ConnectionState::Disconnected);
		};

		DisconnectFromSceneDto dto(sceneHandle);
		dependencyResolver()->resolve<SceneDispatcher>()->removeScene(sceneHandle);
		if (!immediate)
		{
			return sendSystemRequest<DisconnectFromSceneDto, EmptyDto>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, dto).then([taskDisconnected](EmptyDto&) {
				taskDisconnected();
			});
		}
		else
		{
			taskDisconnected();
			return pplx::task_from_result();
		}
	}

	pplx::task<void> Client::disconnectAllScenes(bool immediate)
	{
		//Use TCE because compiler cannot determine when_all return type...
		pplx::task_completion_event<void> tce;
		std::vector<pplx::task<bool>> tasks;
		for (auto it : _scenes)
		{
			auto scene = it.second;
			if (scene)
			{
				tasks.push_back(scene->disconnect(immediate).then([](pplx::task<void> t) {
					try
					{
						t.get();
						return true;
					}
					catch (std::exception&)
					{
						return false;
					}
				}));
			}
		}
		pplx::when_all(std::begin(tasks), std::end(tasks)).then([tce](pplx::task<std::vector<bool>> t) {
			bool success = true;
			std::string reason;
			auto results = t.get();
			for (auto result : results)
			{
				try
				{
					success &= result;
				}
				catch (std::exception& ex)
				{
					success = false;
					if (reason.size())
					{
						reason += "\n";
					}
					reason += ex.what();
				}
			}
			if (!success)
			{
				tce.set_exception(std::runtime_error(reason));
			}
			else
			{
				tce.set();
			}

		});

		return pplx::create_task(tce);
	}

	void Client::transport_packetReceived(Packet_ptr packet)
	{
		for (auto plugin : _plugins)
		{
			plugin->packetReceived(packet);
		}

		this->dependencyResolver()->resolve<IPacketDispatcher>()->dispatchPacket(packet);
	}

	int64 Client::clock()
	{
		return dependencyResolver()->resolve<SyncClock>()->Clock();
	}

	int64 Client::lastPing()
	{
		auto clock = dependencyResolver()->resolve<SyncClock>();
		return clock->LastPing();
	}



	DependencyResolver* Client::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	rxcpp::observable<ConnectionState> Client::GetConnectionStateChangedObservable()
	{
		return _connectionStateObservable.get_observable();
	}

	ConnectionState Client::GetCurrentConnectionState() const
	{
		return (_serverConnection ? _serverConnection->connectionState() : ConnectionState::Disconnected);
	}

	void Client::SetMedatata(const std::string key, const std::string value)
	{
		_metadata[key] = value;
	}

	void Client::dispatchEvent(std::function<void(void)> ev)
	{
		dependencyResolver()->resolve<IActionDispatcher>()->post(ev);
	}
	void Client::setConnectionState(ConnectionState state)
	{
		if (state != _connectionState)
		{
			_connectionState = state;
			auto subscriber = this->_connectionStateObservable.get_subscriber();
			this->dispatchEvent([subscriber, state]() 
			{
				
				subscriber.on_next(state); 
				if (state == ConnectionState::Disconnected)
				{
					subscriber.on_completed();
				}
			});
		}
	}
};
