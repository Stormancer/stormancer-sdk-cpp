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

	Client::Client(Configuration* config)
		: _initialized(false)
		, _accountId(config->account)
		, _applicationName(config->application)
		, _tokenHandler(new TokenHandler())
		, _apiClient(new ApiClient(config, _tokenHandler))
		, _dispatcher(config->dispatcher)
		, _requestProcessor(new RequestProcessor(_logger, std::vector<IRequestModule*>()))
		, _scenesDispatcher(new SceneDispatcher(config->eventDispatcher))
		, _metadata(config->_metadata)
		, _maxPeers(config->maxPeers)
		, _dependencyResolver(new DependencyResolver())
		, _plugins(config->plugins())
		, _synchronisedClock(config->synchronisedClock)
		, _synchronisedClockInterval(config->synchronisedClockInterval)
		, _eventDispatcher(config->eventDispatcher)
	{
		_dependencyResolver->registerDependency<ILogger>(ILogger::instance());
		_dependencyResolver->registerDependency<IScheduler>(config->scheduler);
		_dependencyResolver->registerDependency(config->transportFactory);

		_logger = _dependencyResolver->resolve<ILogger>();
		_scheduler = _dependencyResolver->resolve<IScheduler>();
		_transport = _dependencyResolver->resolve<ITransport>();

		_dispatcher->addProcessor(_requestProcessor);
		_dispatcher->addProcessor(_scenesDispatcher);

		_metadata["serializers"] = "msgpack/array";
		_metadata["transport"] = _transport->name();
		_metadata["version"] = "1.0.0a";
		_metadata["platform"] = "cpp";
		_metadata["protocol"] = "2";

		for (auto plugin : _plugins)
		{
			plugin->clientCreated(this);
		}

		initialize();
	}

	Client::~Client()
	{
#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Client::~Client", "deleting the client...", "");
#endif
		disconnect(true);

		for (auto plugin : _plugins)
		{
			plugin->destroy();
		}
		_plugins.clear();

		delete _scenesDispatcher;
		_scenesDispatcher = nullptr;

		delete _requestProcessor;
		_requestProcessor = nullptr;

		delete _apiClient;
		_apiClient = nullptr;

		delete _tokenHandler;
		_tokenHandler = nullptr;

		if (_serverConnection)
		{
			_serverConnection = nullptr;
		}

		if (_dispatcher)
		{
			delete _dispatcher;
			_dispatcher = nullptr;
		}

		if (_transport)
		{
			delete _transport;
			_transport = nullptr;
		}

#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Client::~Client", "client deleted", "");
#endif
	}

	Client* Client::createClient(Configuration* config)
	{
		if (!config)
		{
#ifdef STORMANCER_LOG_CLIENT
			ILogger::instance()->log(LogLevel::Error, "Client::createClient", "Bad client configuration", "");
#endif
		}

		return new Client(config);
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			_initialized = true;
			_transport->onPacketReceived([this](Packet_ptr packet) { transport_packetReceived(packet); });
			_transport->onTransportEvent([this](void* data, void* data2, void* data3) {
				for (auto plugin : _plugins)
				{
					plugin->transportEvent(data, data2, data3);
				}
			});
			_watch.reset();
		}
	}

	std::string Client::applicationName()
	{
		return _applicationName;
	}

	ILogger* Client::logger()
	{
		return _logger;
	}

	void Client::setLogger(ILogger* logger)
	{
		if (logger != nullptr)
		{
			_logger = logger;
		}
		else
		{
			_logger = ILogger::instance();
		}
	}

	pplx::task<Result<Scene*>*> Client::getPublicScene(const char* sceneId, const char* userData)
	{
		if (!sceneId)
		{
#ifdef STORMANCER_LOG_CLIENT
			ILogger::instance()->log(LogLevel::Error, "Client::getPublicScene", "Bad scene id", "");
#endif
		}

		if (!userData)
		{
			userData = "";
		}

#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Client::getPublicScene", sceneId, userData);
#endif

		return _apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([this, sceneId](pplx::task<SceneEndpoint> t) {
			auto result = new Result<Scene*>();
			try
			{
				SceneEndpoint sep = t.get();
				return this->getScene(sceneId, sep).then([result](pplx::task<Scene*> t) {
					try
					{
						auto scene = t.get();
						result->set(scene);
					}
					catch (const std::exception& ex)
					{
						result->setError(1, ex.what());
					}
					return result;
				});
			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(ex);
				result->setError(1, ex.what());
				return pplx::task<Result<Scene*>*>([result]() {
					return result;
				});
			}
		});
	}

	pplx::task<Result<Scene*>*> Client::getScene(const char* token2)
	{
		std::string token = token2;
		auto sep = _tokenHandler->decodeToken(token);
		return getScene(sep.tokenData.SceneId, sep).then([](pplx::task<Scene*> t) {
			auto result = new Result<Scene*>();
			try
			{
				auto scene = t.get();
				result->set(scene);
			}
			catch (const std::exception& ex)
			{
				result->setError(1, ex.what());
			}
			return result;
		});
	}

	pplx::task<Scene*> Client::getScene(std::string sceneId, SceneEndpoint sep)
	{
		if (_cts.get_token().is_canceled())
		{
			throw std::runtime_error("This Stormancer client has been disconnected and should not be used anymore.");
		}

#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Client::getScene", sceneId.c_str(), sep.token.c_str());
#endif
		if (!_connectionTaskSet)
		{
			_connectionMutex.lock();
			if (!_connectionTaskSet)
			{
				_connectionTask = taskIf(_serverConnection == nullptr, [this, sep]() {

					if (!_transport->isRunning()) {
						try
						{
							_transport->start("client", new ConnectionHandler(), _cts.get_token(), 0, (uint16)(_maxPeers + 1));
							for (auto plugin : _plugins)
							{
								plugin->transportStarted(_transport);
							}
						}
						catch (const std::exception& e)
						{
							throw std::runtime_error(std::string(e.what()) + "\nFailed to start the transport.");
						}
					}
					std::string endpoint = sep.tokenData.Endpoints.at(_transport->name());
#ifdef STORMANCER_LOG_CLIENT
					_logger->log(LogLevel::Trace, "Client::getScene", "Connecting transport", "");
#endif
					try
					{
						this->dispatchEvent([this]() {_onConnectionStateChanged(ConnectionState::Connecting); });
						

						return _transport->connect(endpoint).then([this](IConnection* connection) {
#ifdef STORMANCER_LOG_CLIENT
							_logger->log(LogLevel::Trace, "Client::getScene", "Client::transport connected", "");
#endif
							_serverConnection = connection;

							this->dispatchEvent([this]() {_onConnectionStateChanged(ConnectionState::Connected); });

							auto peerConnectionStateEraseIterator = new Action<ConnectionState>::TIterator();
							*peerConnectionStateEraseIterator = _serverConnection->onConnectionStateChanged([this, peerConnectionStateEraseIterator](ConnectionState connectionState) {
								if (connectionState == ConnectionState::Disconnected)
								{
									_serverConnection->connectionStateChangedAction().erase(*peerConnectionStateEraseIterator);
									_serverConnection = nullptr;
									delete peerConnectionStateEraseIterator;
								}
								auto scenes = _scenes;
								for (auto sceneIt : scenes)
								{
									auto scene = sceneIt.second;
									for (auto plugin : _plugins)
									{
										plugin->sceneDisconnected(scene);
									}
									this->dispatchEvent([scene]() {scene->setConnectionState(ConnectionState::Disconnected); });
									
								}
								this->dispatchEvent([this,connectionState]() {_onConnectionStateChanged(connectionState); });
								;
							});

							_serverConnection->setMetadata(_metadata);
						});
					}
					catch (const std::exception& ex)
					{
						throw std::runtime_error(std::string(ex.what()) + "\nFailed to connect the transport.");
					}
				}).then([this, sep]() {
					return updateServerMetadata();
				}).then([this, sep]() {
					if (_synchronisedClock && sep.tokenData.Version > 0)
					{
						startSyncClock();
					}
				});
			}

			_connectionTaskSet = true;
			_connectionMutex.unlock();
		}

		return _connectionTask.then([this, sep](){
			SceneInfosRequestDto parameter;
			parameter.Metadata = _serverConnection->metadata();
			parameter.Token = sep.token;
#ifdef STORMANCER_LOG_CLIENT
			_logger->log(LogLevel::Trace, "Client::getScene", "send SceneInfosRequestDto", "");
#endif
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
#ifdef STORMANCER_LOG_CLIENT
			_logger->log(LogLevel::Trace, "Client::getScene", "SceneInfosDto received", ss.str().c_str());
#endif
			_serverConnection->metadata()["serializer"] = sceneInfos.SelectedSerializer;

			return updateServerMetadata().then([this, sep, sceneId, sceneInfos]() {
#ifdef STORMANCER_LOG_CLIENT
				_logger->log(LogLevel::Trace, "Client::getScene", "Returning the scene ", sceneId.c_str());
#endif
				Scene* scene;
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
					scene = new Scene(_serverConnection, this, sceneId, sep.token, sceneInfos, _dependencyResolver, sceneDeleter);
					_scenes[sceneId] = scene;
				}
				for (auto plugin : _plugins)
				{
					plugin->sceneCreated(scene);
				}
				return scene;
			});
		});
	}

	pplx::task<void> Client::updateServerMetadata()
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Client::updateServerMetadata", "sending system request.", "");
#endif
		return _requestProcessor->sendSystemRequest(_serverConnection, (byte)SystemRequestIDTypes::ID_SET_METADATA, [this](bytestream* bs) {
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

	pplx::task<void> Client::connectToScene(Scene* scene, std::string& token, std::vector<Route_ptr> localRoutes)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		if (scene->connectionState() != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not in disconnected state");
		}

		for (auto plugin : _plugins)
		{
			plugin->sceneConnecting(scene);
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
			_scenesDispatcher->addScene(scene);
			for (auto plugin : _plugins)
			{
				plugin->sceneConnected(scene);
			}
			scene->setConnectionState(ConnectionState::Connected);
#ifdef STORMANCER_LOG_CLIENT
			ILogger::instance()->log(LogLevel::Info, "client", "Scene connected", scene->id());
#endif
		});
	}

	pplx::task<Result<>*> Client::disconnect(bool immediate)
	{
		pplx::task_completion_event<Result<>*> tce;
		auto result = new Result<>();

		try
		{
			stopSyncClock();

			if (_serverConnection && _serverConnection->connectionState() == ConnectionState::Connected)
			{
				// Stops the synchronised clock, disconnect the scenes, closes the server connection then stops the underlying transports.

				auto dt = disconnectAllScenes(immediate);

				auto end = [this, tce, result]() {
					_cts.cancel(); // this will cause the transport to stop

					result->set();
					tce.set(result);

					if (_serverConnection)
					{
						_serverConnection->close();
					}
				};

				if (!immediate)
				{
					dt.then([tce, result, end](pplx::task<void> t) {
						try
						{
							t.wait();
							end();
						}
						catch (const std::exception& ex)
						{
							result->setError(1, ex.what());
							tce.set(result);
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
				result->set();
				tce.set(result);
			}
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
			tce.set(result);
		}

		return pplx::create_task(tce);
	}

	pplx::task<void> Client::disconnect(Scene* scene, byte sceneHandle, bool immediate)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is null");
		}

		if (scene->connectionState() != ConnectionState::Connected)
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
		_scenesDispatcher->removeScene(sceneHandle);
		if (!immediate)
		{
			return sendSystemRequest<DisconnectFromSceneDto, EmptyDto>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, dto).then([taskDisconnected](EmptyDto&) {
				taskDisconnected();
			});
		}
		else
		{
			taskDisconnected();
			return taskCompleted();
		}
	}

	pplx::task<void> Client::disconnectAllScenes(bool immediate)
	{
		pplx::task_completion_event<void> tce;

		std::vector<pplx::task<Result<>*>> tasks;
		for (auto it : _scenes)
		{
			auto scene = it.second;
			if (scene)
			{
				tasks.push_back(scene->disconnect(immediate));
			}
		}
		pplx::when_all(std::begin(tasks), std::end(tasks)).then([tce](std::vector<Result<>*> results) {
			bool success = true;
			std::string reason;
			for (auto result : results)
			{
				if (!result->success())
				{
					success = false;
					if (reason.size())
					{
						reason += "\n";
					}
					reason += result->reason();
				}
				delete result;
			}

			if (success)
			{
				tce.set();
			}
			else
			{
				tce.set_exception<std::runtime_error>(std::runtime_error(reason));
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

		this->_dispatcher->dispatchPacket(packet);
	}

	int64 Client::clock()
	{
		return (int64)(_watch.getElapsedTime() + _offset);
	}

	int64 Client::lastPing()
	{
		return _lastPing;
	}

	void Client::startSyncClock()
	{
		ILogger::instance()->log(LogLevel::Trace, "Client::startSyncClock", "Starting syncClock...", "");
		std::lock_guard<std::mutex> lg(_syncClockMutex);
		if (_scheduler)
		{
			int interval = (int)(_clockValues.size() >= _maxClockValues ? _synchronisedClockInterval : _pingIntervalAtStart);
			_syncClockSubscription = _scheduler->schedulePeriodic(interval, [this, interval]() {
				std::lock_guard<std::mutex> lg(_syncClockMutex);
				if (_syncClockSubscription && _syncClockSubscription->subscribed())
				{
					if (interval == _pingIntervalAtStart && _clockValues.size() >= _maxClockValues)
					{
						pplx::task<void>([this]() {
							stopSyncClock();
							startSyncClock();
						});
					}
					else
					{
						syncClockImpl();
					}
				}
			});
		}
		syncClockImpl();
		ILogger::instance()->log(LogLevel::Trace, "Client::startSyncClock", "SyncClock started", "");
	}

	void Client::stopSyncClock()
	{
		ILogger::instance()->log(LogLevel::Trace, "Client::stopSyncClock", "waiting to stop syncClock...", "");
		std::lock_guard<std::mutex> lg(_syncClockMutex);
		if (_syncClockSubscription)
		{
			if (_syncClockSubscription->subscribed())
			{
				_syncClockSubscription->unsubscribe();
			}
			_syncClockSubscription->destroy();
			_syncClockSubscription = nullptr;
		}
		ILogger::instance()->log(LogLevel::Trace, "Client::stopSyncClock", "SyncClock stopped", "");
	}

	pplx::task<void> Client::syncClockImpl()
	{
		if (!lastPingFinished)
		{
			return taskCompleted();
		}

		try
		{
			lastPingFinished = false;

			uint64 timeStart = _watch.getElapsedTime();
			return _requestProcessor->sendSystemRequest(_serverConnection, (byte)SystemRequestIDTypes::ID_PING, [&timeStart](bytestream* bs) {
				*bs << timeStart;
			}, PacketPriority::IMMEDIATE_PRIORITY).then([this, timeStart](pplx::task<Packet_ptr> t) {
				Packet_ptr packet;
				try
				{
					packet = t.get();
				}
				catch (const std::exception& ex)
				{
#ifdef STORMANCER_LOG_CLIENT
					ILogger::instance()->log(LogLevel::Warn, "syncClock", "Failed to ping the server", ex.what());
#endif
					return;
				}

				uint64 timeEnd = (uint16)this->_watch.getElapsedTime();

				lastPingFinished = true;

				uint64 timeServer;
				*packet->stream >> timeServer;

				uint16 ping = (uint16)(timeEnd - timeStart);
				_lastPing = ping;
				double latency = ping / 2.0;

				double offset = timeServer - timeEnd + latency;

				_clockValues.push_back(ClockValue{ latency, offset });
				if (_clockValues.size() > _maxClockValues)
				{
					_clockValues.pop_front();
				}
				auto len = _clockValues.size();

				std::vector<double> latencies(len);
				for (auto i = 0; i < len; i++)
				{
					latencies[i] = _clockValues[i].latency;
				}
				std::sort(latencies.begin(), latencies.end());
				_medianLatency = latencies[len / 2];
				double pingAvg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / len;
				double varianceLatency = 0;
				for (auto v : latencies)
				{
					auto tmp = v - pingAvg;
					varianceLatency += (tmp * tmp);
				}
				varianceLatency /= len;
				_standardDeviationLatency = std::sqrt(varianceLatency);

				double offsetsAvg = 0;
				uint32 lenOffsets = 0;
				double latencyMax = _medianLatency + _standardDeviationLatency;
				for (auto v : _clockValues)
				{
					if (v.latency < latencyMax)
					{
						offsetsAvg += v.offset;
						lenOffsets++;
					}
				}
				_offset = offsetsAvg / lenOffsets;
			});
		}
		catch (const std::exception& ex)
		{
#ifdef STORMANCER_LOG_CLIENT
			_logger->log(LogLevel::Error, "Client::syncClockImpl", "Failed to ping server.", ex.what());
#endif
			throw std::runtime_error(std::string() + ex.what() + "\nFailed to ping server.");
		}
	}

	DependencyResolver* Client::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	Action<ConnectionState>& Client::connectionStateChangedAction()
	{
		return _onConnectionStateChanged;
	}

	Action<ConnectionState>::TIterator Client::onConnectionStateChanged(std::function<void(ConnectionState)> callback)
	{
		return _onConnectionStateChanged.push_back(callback);
	}

	ConnectionState Client::connectionState() const
	{
		return (_serverConnection ? _serverConnection->connectionState() : ConnectionState::Disconnected);
	}

	void Client::SetMedatata(const char* key, const char* value)
	{
		_metadata[key] = value;
	}

	void Client::dispatchEvent(std::function<void(void)> ev)
	{
		_eventDispatcher(ev);
	}
};
