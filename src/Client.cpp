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
		: _initialized(false),
		_accountId(config->account),
		_applicationName(config->application),
		_tokenHandler(new TokenHandler()),
		_apiClient(new ApiClient(config, _tokenHandler)),
		_dispatcher(config->dispatcher),
		_requestProcessor(new RequestProcessor(_logger, std::vector<IRequestModule*>())),
		_scenesDispatcher(new SceneDispatcher),
		_metadata(config->_metadata),
		_maxPeers(config->maxPeers),
		_dependencyResolver(new DependencyResolver()),
		_plugins(config->plugins()),
		_synchronisedClock(config->synchronisedClock),
		_synchronisedClockInterval(config->synchronisedClockInterval)
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
		ILogger::instance()->log(LogLevel::Warn, "Client destructor", "deleting the client...", "");
		disconnect();

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
			delete _serverConnection;
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
	}

	Client* Client::createClient(Configuration* config)
	{
		if (!config)
		{
			throw std::invalid_argument("Check the configuration");
		}

		return new Client(config);
	}

	void Client::destroy()
	{
		delete this;
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			_initialized = true;
			_transport->packetReceived += [this](Packet_ptr packet) {
				this->transport_packetReceived(packet);
			};
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
			throw std::invalid_argument("Bad scene id");
		}

		if (!userData)
		{
			userData = "";
		}

		_logger->log(LogLevel::Trace, "Client::getPublicScene", sceneId, userData);

		return _apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([this, sceneId](pplx::task<SceneEndpoint> t) {
			try
			{
				SceneEndpoint sep = t.get();
				return this->getScene(sceneId, sep).then([](pplx::task<Scene*> t) {
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
			catch (const std::exception& e)
			{
				std::runtime_error error(std::string(e.what()) + "\nUnable to get the scene endpoint.");
				_logger->log(error);
				throw error;
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
		_logger->log(LogLevel::Trace, "Client::getScene", sceneId.c_str(), sep.token.c_str());

		return taskIf(_serverConnection == nullptr, [this, sep]() {
			if (!_transport->isRunning()) {
				_cts = pplx::cancellation_token_source();
				try
				{
					_transport->start("client", new ConnectionHandler(), _cts.get_token(), 0, (uint16)(_maxPeers + 1));
				}
				catch (const std::exception& e)
				{
					throw std::runtime_error(std::string(e.what()) + "\nFailed to start the transport.");
				}
			}
			std::string endpoint = sep.tokenData.Endpoints.at(_transport->name());
			_logger->log(LogLevel::Trace, "Client::getScene", "Connecting transport", "");
			try
			{
				return _transport->connect(endpoint).then([this](pplx::task<IConnection*> t) {
					try
					{
						IConnection* connection = t.get();
						_logger->log(LogLevel::Trace, "Client::getScene", "Client::transport connected", "");
						_serverConnection = connection;
						_serverConnection->metadata = _metadata;
					}
					catch (const std::exception& e)
					{
						throw std::runtime_error(std::string(e.what()) + "\nFailed to get the transport peer connection.");
					}
				});
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string(e.what()) + "\nFailed to connect the transport.");
			}
		}).then([this, sep](pplx::task<void> t) {
			try
			{
				t.wait();
				return updateServerMetadata();
			}
			catch (const std::exception& e)
			{
				throw e;
			}
		}).then([this, sep](pplx::task<void> t) {
			try
			{
				t.wait();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string(e.what()) + "\nFailed to start the sync clock.");
			}

			if (_synchronisedClock && sep.tokenData.Version > 0)
			{
				startSyncClock();
			}
			SceneInfosRequestDto parameter;
			parameter.Metadata = _serverConnection->metadata;
			parameter.Token = sep.token;
			_logger->log(LogLevel::Trace, "Client::getScene", "send SceneInfosRequestDto", "");
			return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter);
		}).then([this, sep, sceneId](pplx::task<SceneInfosDto> t) {
			SceneInfosDto result;
			try
			{
				result = t.get();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string(e.what()) + "\nFailed to get scene infos.");
			}

			std::stringstream ss;
			ss << result.SceneId << " " << result.SelectedSerializer << " Routes:[";
			for (uint32 i = 0; i < result.Routes.size(); i++)
			{
				ss << result.Routes.at(i).Handle << ":" << result.Routes.at(i).Name << ";";
			}
			ss << "] Metadata:[";
			for (auto it : result.Metadata)
			{
				ss << it.first << ":" << it.second << ";";
			}
			ss << "]";
			_logger->log(LogLevel::Trace, "Client::getScene", "SceneInfosDto received", ss.str().c_str());
			_serverConnection->metadata["serializer"] = result.SelectedSerializer;
			return updateServerMetadata().then([this, sep, sceneId, result](pplx::task<void> t) {
				try
				{
					t.wait();

					_logger->log(LogLevel::Trace, "Client::getScene", "Returning the scene ", sceneId.c_str());
					Scene* scene;
					if (mapContains(_scenes, sceneId))
					{
						scene = _scenes[sceneId];
					}
					else
					{
						scene = new Scene(_serverConnection, this, sceneId, sep.token, result, _dependencyResolver, [this, sceneId]() {
							if (mapContains(_scenes, sceneId))
							{
								_scenes.erase(sceneId);
							}
						});
						_scenes[sceneId] = scene;
					}
					for (auto plugin : _plugins)
					{
						plugin->sceneCreated(scene);
					}
					return scene;
				}
				catch (const std::exception& e)
				{
					throw e;
				}
			});
		});
	}

	pplx::task<void> Client::updateServerMetadata()
	{
		_logger->log(LogLevel::Trace, "Client::updateServerMetadata", "sending system request.", "");
		return _requestProcessor->sendSystemRequest(_serverConnection, (byte)SystemRequestIDTypes::ID_SET_METADATA, [this](bytestream* bs) {
			msgpack::pack(bs, _serverConnection->metadata);
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

		for (auto plugin : _plugins)
		{
			plugin->sceneConnecting(scene);
		}

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
		parameter.ConnectionMetadata = _serverConnection->metadata;

		return Client::sendSystemRequest<ConnectToSceneMsg, ConnectionResult>((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter).then([this, scene](pplx::task<ConnectionResult> t) {
			try
			{
				auto result = t.get();
				scene->completeConnectionInitialization(result);
				_scenesDispatcher->addScene(scene);
				for (auto plugin : _plugins)
				{
					plugin->sceneConnected(scene);
				}
			}
			catch (const std::exception& e)
			{
				auto e2 = std::runtime_error(std::string(e.what()) + "\nFailed to connect to the scene.");
				_logger->log(e2);
				throw e2;
			}
		});
	}

	void Client::disconnect()
	{
		_cts.cancel();

		if (_synchronisedClock)
		{
			stopSyncClock();
		}

		disconnectAllScenes();

		if (_serverConnection)
		{
			_serverConnection->close();
		}
	}

	pplx::task<void> Client::disconnect(Scene* scene, byte sceneHandle)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is null");
		}

		DisconnectFromSceneDto dto(sceneHandle);
		for (auto plugin : _plugins)
		{
			plugin->sceneDisconnected(scene);
		}
		_scenesDispatcher->removeScene(sceneHandle);
		return sendSystemRequest<DisconnectFromSceneDto, EmptyDto>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, dto).then([](EmptyDto&){});
	}

	void Client::disconnectAllScenes()
	{
		for (auto it : _scenes)
		{
			auto scene = it.second;
			if (scene)
			{
				scene->disconnect();
			}
		}
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
		if (_scheduler)
		{
			auto delay = (_clockValues.size() < _maxClockValues ? _pingIntervalAtStart : _synchronisedClockInterval);
			_syncClockSubscription = _scheduler->schedulePeriodic((int)delay, [this, delay]() {
				if (delay == _pingIntervalAtStart && _clockValues.size() >= _maxClockValues)
				{
					stopSyncClock();
					startSyncClock();
				}
				else
				{
					syncClockImpl();
				}
			});
		}
		syncClockImpl();
	}

	void Client::stopSyncClock()
	{
		_syncClockSubscription->unsubscribe();
	}

	pplx::task<void> Client::syncClockImpl()
	{
		bool skipPing;
		_syncClockMutex.lock();
		skipPing = !lastPingFinished;
		_syncClockMutex.unlock();

		if (skipPing)
		{
			return taskCompleted();
		}

		try
		{
			_syncClockMutex.lock();
			lastPingFinished = false;
			_syncClockMutex.unlock();

			uint64 timeStart = _watch.getElapsedTime();
			return _requestProcessor->sendSystemRequest(_serverConnection, (byte)SystemRequestIDTypes::ID_PING, [&timeStart](bytestream* bs) {
				*bs << timeStart;
			}, PacketPriority::IMMEDIATE_PRIORITY).then([this, timeStart](pplx::task<Packet_ptr> t) {
				Packet_ptr packet;
				try
				{
					packet = t.get();
				}
				catch (const std::exception& e)
				{
					ILogger::instance()->log(LogLevel::Warn, "syncClock", "Failed to ping the server", e.what());
					return;
				}

				uint64 timeEnd = (uint16)this->_watch.getElapsedTime();

				_syncClockMutex.lock();

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

				_syncClockMutex.unlock();
			});
		}
		catch (std::exception e)
		{
			_logger->log(LogLevel::Error, "Client::syncClockImpl", "Failed to ping server.", "");
			throw std::runtime_error("Failed to ping server.");
		}
	}

	DependencyResolver* Client::dependencyResolver() const
	{
		return _dependencyResolver;
	}
};
