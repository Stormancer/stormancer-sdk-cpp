#include "stormancer/stdafx.h"
#include "stormancer/Shutdown.h"
#include "stormancer/Client.h"
#include "stormancer/ContainerConfigurator.h"
#include "stormancer/DependencyResolver.h"
#include "stormancer/ApiClient.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/TokenHandler.h"
#include "stormancer/SyncClock.h"
#include "stormancer/SceneInfosRequestDto.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/ConnectToSceneMsg.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/KeyStore.h"
















namespace
{













}

namespace Stormancer
{
	// Client

	Client::Client(Configuration_ptr config):
		 _initialized(false)
		, _accountId(config->account)
		, _applicationName(config->application)
		, _maxPeers(config->maxPeers)
		, _metadata(config->_metadata)
		, _plugins(config->plugins())
		, _config(config)
	{
		
	}

	Client::~Client()
	{
		_cts.cancel();

		if (_connectionSubscription.is_subscribed())
		{
			_connectionSubscription.unsubscribe();
		}
		for (auto plugin : _plugins)
		{
			delete plugin;
		}
		_plugins.clear();

		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			_scenes.clear();
		}
	}

	Client_ptr Client::create(Configuration_ptr config)
	{
		if (!config)
		{
			return nullptr;
		}

		Client_ptr client(new Client(config), [](Client* ptr) { delete ptr; });
		
		return client;
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			auto dr = std::make_shared<DependencyResolver>();
			ConfigureContainer(dr, _config);
			_dependencyResolver = dr;

			logger()->log(LogLevel::Trace, "Client", "Creating the client...");

			std::vector<std::shared_ptr<IRequestModule>> modules{ std::dynamic_pointer_cast<IRequestModule>(_dependencyResolver->resolve<P2PRequestModule>()) };

			RequestProcessor::Initialize(_dependencyResolver->resolve<RequestProcessor>(), modules);


































			logger()->log(LogLevel::Trace, "Client", "Client created");

			logger()->log(LogLevel::Trace, "Client", "Initializing client...");
			auto transport = _dependencyResolver->resolve<ITransport>();

			_metadata["serializers"] = "msgpack/array";
			_metadata["transport"] = transport->name();
			_metadata["version"] = "1.2.0";



			_metadata["platform"] = __PLATFORM__;

			_metadata["protocol"] = "2";

			for (auto plugin : _plugins)
			{
				plugin->clientCreated(this);
			}
			auto actionDispatcher = _dependencyResolver->resolve<IActionDispatcher>();
			actionDispatcher->start();
			_disconnectionTce = pplx::task_completion_event<void>();
			_initialized = true;
			_dependencyResolver->resolve<IActionDispatcher>()->start();
			transport->onPacketReceived(createSafeCapture(STRM_WEAK_FROM_THIS(), [this](Packet_ptr packet) {
				transport_packetReceived(packet);
			}));
			_cts = pplx::cancellation_token_source();
			logger()->log(LogLevel::Trace, "Client", "Client initialized");
		}
	}

	void Client::clear()
	{
		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		_initialized = false;
		_metadata.clear();
		_dependencyResolver = nullptr;
		this->_initialized = false;
		
		_connectionSubscription.unsubscribe();
	}

	const std::string& Client::applicationName() const
	{
		return _applicationName;
	}

	ILogger_ptr Client::logger() const
	{
		auto result = _dependencyResolver->resolve<ILogger>();
		if (result)
		{
			return result;
		}
		else
		{
			return nullptr;
		}
	}

	pplx::task<Scene_ptr> Client::connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer, pplx::cancellation_token ct)
	{
		
		return getPublicScene(sceneId, ct)
			.then([initializer, ct](Scene_ptr scene)
		{
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}
			}
			return scene->connect(ct)
				.then([scene]()
			{
				return scene;
			}, ct);
		}, ct);
	}

	pplx::task<Scene_ptr> Client::connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer, pplx::cancellation_token ct)
	{
		
		return getPrivateScene(sceneToken, ct)
			.then([initializer, ct](Scene_ptr scene)
		{
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}
			}
			return scene->connect(ct)
				.then([scene]()
			{
				return scene;
			}, ct);
		}, ct);
	}

	pplx::task<Scene_ptr> Client::getConnectedScene(const std::string& sceneId, pplx::cancellation_token ct)
	{
		logger()->log(LogLevel::Trace, "Client", "Get connected scene.", sceneId);

		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "SceneId is empty");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("SceneId is empty"));
		}

		std::lock_guard<std::mutex> lg(_scenesMutex);
		if (mapContains(_scenes, sceneId))
		{
			return _scenes[sceneId].task;
		}

		return pplx::task_from_result<Scene_ptr>(nullptr);
	}

	std::vector<std::string> Client::getSceneIds()
	{
		std::vector<std::string> sceneIds;
		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			for (auto it : _scenes)
			{
				sceneIds.emplace_back(it.first);
			}
		}
		return sceneIds;
	}

	pplx::task<Scene_ptr> Client::getPublicScene(const std::string& sceneId, pplx::cancellation_token ct)
	{
		if (this->getConnectionState() == ConnectionState::Disconnecting)
		{
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Client disconnecting"));
		}
		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		this->initialize();
		logger()->log(LogLevel::Trace, "Client", "Get public scene.", sceneId);

		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Bad scene id.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Bad scene id."));
		}

		
		if (mapContains(_scenes, sceneId))
		{
			if (_scenes[sceneId].isPublic)
			{
				return _scenes[sceneId].task;
			}
			else
			{
				logger()->log(LogLevel::Error, "Client", "The scene is private.");
				return pplx::task_from_exception<Scene_ptr>(std::runtime_error("The scene is private."));
			}
		}
		else
		{
			ClientScene cScene;
			cScene.isPublic = true;

			auto task = ensureNetworkAvailable()
				.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, sceneId, ct]()
			{
				auto apiClient = _dependencyResolver->resolve<ApiClient>();
				return apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, ct);
			}), ct)
				.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, sceneId, ct](SceneEndpoint sep)
			{
				return getSceneInternal(sceneId, sep, ct);
			}), ct);

			cScene.task = task;
			_scenes[sceneId] = cScene;
			return task;
		}
	}

	pplx::task<Scene_ptr> Client::getPrivateScene(const std::string& sceneToken, pplx::cancellation_token ct)
	{
		if (this->getConnectionState() == ConnectionState::Disconnecting)
		{
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Client disconnecting"));
		}

		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		this->initialize();
		logger()->log(LogLevel::Trace, "Client", "Get private scene.", sceneToken);

		if (sceneToken.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Bad scene token.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Bad scene token."));
		}

		auto tokenHandler = _dependencyResolver->resolve<ITokenHandler>();
		auto sep = tokenHandler->decodeToken(sceneToken);
		auto sceneId = sep.tokenData.SceneId;

		if (mapContains(_scenes, sceneId))
		{
			return _scenes[sceneId].task;
		}
		else
		{
			ClientScene cScene;
			cScene.isPublic = true;
			auto task = getSceneInternal(sep.tokenData.SceneId, sep, ct);
			cScene.task = task;
			_scenes[sceneId] = cScene;
			return task;
		}
	}

	pplx::task<void> Client::ensureConnectedToServer(const SceneEndpoint& endpoint, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		if (_connectionState == ConnectionState::Disconnected)
		{
			std::lock_guard<std::mutex> lock(_connectionMutex);
			if (_connectionState == ConnectionState::Disconnected)
			{
				setConnectionState(ConnectionState::Connecting);

				try
				{
					//Start transport and execute plugin event
					logger()->log(LogLevel::Trace, "Client", "Starting transport", "port:" + std::to_string(_config->clientSDKPort) + "; maxPeers:" + std::to_string((uint16)_maxPeers + 1));
					auto transport = _dependencyResolver->resolve<ITransport>();
					transport->start("client", _dependencyResolver->resolve<IConnectionManager>(), _cts.get_token(), _config->clientSDKPort, (uint16)_maxPeers + 1);
					for (auto plugin : _plugins)
					{
						plugin->transportStarted(transport.get());
					}

					//Connect to server
					std::string endpointUrl;
					if (endpoint.version == 1)
					{
						endpointUrl = endpoint.tokenData.Endpoints.at(transport->name());
					}
					else
					{
						auto& endpoints = endpoint.getTokenResponse.endpoints.at(transport->name());
						endpointUrl = endpoints.at(std::rand() % endpoints.size());
						if (_config->encryptionEnabled)
						{
							_metadata["encryption"] = endpoint.getTokenResponse.encryption.token;
							auto keyStore = _dependencyResolver->resolve<KeyStore>();
							auto key = utility::conversions::from_base64(utility::string_t(endpoint.getTokenResponse.encryption.key.begin(), endpoint.getTokenResponse.encryption.key.end()));
							if (key.size() != 256 / 8)
							{
								throw std::runtime_error("Unexpected key size. received " + std::to_string(key.size() * 8) + " bits expected 256 bits ");
							}
							std::copy(key.begin(), key.end(), keyStore->key);
						}
					}

					if (this->_config->forceTransportEndpoint != "")
					{
						endpointUrl = this->_config->forceTransportEndpoint;
					}
					logger()->log(LogLevel::Trace, "Client", "Connecting transport to server", endpointUrl);
					_connectionTask = transport->connect(endpointUrl, ct)
						.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this](std::weak_ptr<IConnection> connectionWeak)
					{
						auto connection = connectionWeak.lock();
						if (!connection)
						{
							throw std::runtime_error("Connection not available");
						}

						auto onConnectionStateChangedNext = createSafeCaptureNoThrow(STRM_WEAK_FROM_THIS(), [this](ConnectionState state) {
							if (state == ConnectionState::Disconnecting)
							{
								std::lock_guard<std::mutex> lg(_scenesMutex);
								for (auto sceneIt : _scenes)
								{
									sceneIt.second.task
										.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this](Scene_ptr scene)
									{
										dispatchEvent([scene]() {
											scene->setConnectionState(ConnectionState::Disconnecting);
										});
										for (auto plugin : _plugins)
										{
											plugin->sceneDisconnecting(scene.get());
										}
									}))
										.then([](pplx::task<void> t)
									{
										try
										{
											t.get();
										}
										catch (const std::exception&)
										{
											// catch any exception
										}
									});
								}
							}
							else if (state == ConnectionState::Disconnected)
							{
								std::lock_guard<std::mutex> lg(_scenesMutex);
								for (auto sceneIt : _scenes)
								{
									sceneIt.second.task
										.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this](Scene_ptr scene)
									{
										dispatchEvent([scene]() {
											scene->setConnectionState(ConnectionState::Disconnected);
										});
										for (auto plugin : _plugins)
										{
											plugin->sceneDisconnected(scene.get());
										}
									})).then([](pplx::task<void> t) {
										try
										{
											t.wait();
										}
										catch (const std::exception&)
										{
											// catch any exception
										}
									});
								}
								_cts.cancel(); // this will cause the syncClock and the transport to stop
								_serverConnection.reset();
							}

							setConnectionState(state);
						});

						auto loggerPtr = logger();
						auto onConnectionStateChangedError = [loggerPtr](std::exception_ptr exptr) {
							try
							{
								std::rethrow_exception(exptr);
							}
							catch (const std::exception& ex)
							{
								loggerPtr->log(LogLevel::Error, "Client", "Connection state change failed", ex.what());
							}
						};

						_connectionSubscription = connection->getConnectionStateChangedObservable().subscribe(onConnectionStateChangedNext, onConnectionStateChangedError);

						setConnectionState(connection->getConnectionState());

						// Flatten all external addresses into a comma-separated string
						/*std::stringstream addrs;
						auto transport = _dependencyResolver->resolve<ITransport>();
						for (const std::string& addr : transport->externalAddresses())
						{
							addrs << addr << ",";
						}
						_metadata["externalAddrs"] = addrs.str();
						_metadata["externalAddrs"].pop_back();*/

						connection->setMetadata(_metadata);
						_serverConnection = connectionWeak;
						return;
					}), ct)
						.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, ct]()
					{
						return updateServerMetadata(ct);
					}), ct)
						.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, endpoint]()
					{
						if (_config->synchronisedClock)
						{
							_dependencyResolver->resolve<SyncClock>()->start(_serverConnection, _cts.get_token());
						}
					}), ct)
						.then(createSafeCaptureNoThrow(STRM_WEAK_FROM_THIS(), [this](pplx::task<void> task)
					{
						try
						{
							task.get();
						}
						catch (...)
						{
							if (auto connection = _serverConnection.lock())
							{
								connection->close();
							}
							setConnectionState(ConnectionState::Disconnected);
							throw;
						}
					})); // no ct here because we need this continuation to awalys run
				}
				catch (std::exception& ex)
				{
					setConnectionState(ConnectionState::Disconnected);
					return pplx::task_from_exception<void>(std::runtime_error(std::string("Failed to start transport: ") + ex.what()));
				}
			}
		}

		return _connectionTask;
	}

	pplx::task<Scene_ptr> Client::getSceneInternal(const std::string& sceneId, const SceneEndpoint& sep, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		logger()->log(LogLevel::Trace, "Client", "Get scene " + sceneId, sep.token);

		return ensureConnectedToServer(sep, ct)
			.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, sep, ct]()
		{
			SceneInfosRequestDto parameter;
			auto serverConnection = _serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("Connection not available");
			}
			parameter.Metadata = serverConnection->metadata();
			parameter.Token = sep.token;

			logger()->log(LogLevel::Trace, "Client", "Send SceneInfosRequestDto");
			return sendSystemRequest<SceneInfosDto>((byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter, ct);
		}), ct)
			.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, ct](SceneInfosDto sceneInfos)
		{
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

			logger()->log(LogLevel::Trace, "Client", "SceneInfosDto received", ss.str());

			auto serverConnection = _serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("Connection not available");
			}
			serverConnection->setMetadata("serializer", sceneInfos.SelectedSerializer);

			return updateServerMetadata(ct)
				.then([sceneInfos]()
			{
				return sceneInfos;
			}, ct);
		}), ct)
			.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, sceneId, sep](SceneInfosDto sceneInfos)
		{
			logger()->log(LogLevel::Trace, "Client", "Return the scene", sceneId);
			Scene_ptr scene(new Scene(_serverConnection, STRM_WEAK_FROM_THIS(), sceneId, sep.token, sceneInfos, _dependencyResolver), [](Scene* ptr) { delete ptr; });
			scene->initialize();
			for (auto plugin : _plugins)
			{
				plugin->registerSceneDependencies(scene.get());
			}
			for (auto plugin : _plugins)
			{
				plugin->sceneCreated(scene.get());
			}
			return scene;
		}), ct);
	}

	pplx::task<void> Client::updateServerMetadata(pplx::cancellation_token ct)
	{
		auto loggerPtr = logger();
		loggerPtr->log(LogLevel::Trace, "Client", "Update server metadata");

		ct = getLinkedCancellationToken(ct);

		auto serverConnection = _serverConnection.lock();
		if (!serverConnection)
		{
			throw std::runtime_error("Connection not available");
		}

		auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();

		return requestProcessor->sendSystemRequest(serverConnection.get(), (byte)SystemRequestIDTypes::ID_SET_METADATA, createSafeCapture(STRM_WEAK_FROM_THIS(), [this, serverConnection](obytestream* stream) {
			_serializer.serialize(stream, serverConnection->metadata());
		}), PacketPriority::MEDIUM_PRIORITY, ct)
			.then([loggerPtr](Packet_ptr)
		{
			loggerPtr->log(LogLevel::Trace, "Client", "Updated server metadata");
		}, ct);
	}

	pplx::task<void> Client::connectToScene(const std::string& sceneId, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		std::lock_guard<std::mutex> lg(_scenesMutex);

		if (!mapContains(_scenes, sceneId))
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not found."));
		}

		auto& cScene = _scenes[sceneId];

		if (cScene.connecting)
		{
			return cScene.task.then([](Scene_ptr) {});
		}

		cScene.connecting = true;
		auto& task = cScene.task;

		task = task
			.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, sceneToken, localRoutes, ct](Scene_ptr scene)
		{
			if (scene->getCurrentConnectionState() != ConnectionState::Disconnected)
			{
				throw std::runtime_error("The scene is not in disconnected state.");
			}

			scene->setConnectionState(ConnectionState::Connecting);

			for (auto plugin : _plugins)
			{
				plugin->sceneConnecting(scene.get());
			}

			ConnectToSceneMsg parameter;
			parameter.Token = sceneToken;
			for (auto r : localRoutes)
			{
				RouteDto routeDto;
				routeDto.Handle = r->handle();
				routeDto.Metadata = r->metadata();
				routeDto.Name = r->name();
				parameter.Routes << routeDto;
			}

			auto serverConnection = _serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("Connection not available");
			}
			parameter.ConnectionMetadata = serverConnection->metadata();

			return sendSystemRequest<ConnectionResult>((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter, ct)
				.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [this, scene](ConnectionResult result)
			{
				scene->completeConnectionInitialization(result);
				auto sceneDispatcher = _dependencyResolver->resolve<SceneDispatcher>();
				sceneDispatcher->addScene(scene);
				scene->setConnectionState(ConnectionState::Connected);
				for (auto plugin : _plugins)
				{
					plugin->sceneConnected(scene.get());
				}
				logger()->log(LogLevel::Debug, "client", "Scene connected.", scene->id());
				return scene;
			}), ct);
		}), ct);

		return task.then([](Scene_ptr) {}, ct);
	}

	pplx::task<void> Client::disconnect()
	{
		bool proceedToDisconnection = false;

		{
			std::lock_guard<std::mutex> lg(_connectionMutex);
			if (_connectionState == ConnectionState::Connected)
			{
				proceedToDisconnection = true;
				setConnectionState(ConnectionState::Disconnecting);
			}
			else if (_connectionState == ConnectionState::Connecting)
			{
				return pplx::task_from_exception<void>(std::runtime_error("Client is connecting"));
			}
			else if (_connectionState == ConnectionState::Disconnected)
			{
				return pplx::task_from_result();
			}
		}

		if (proceedToDisconnection)
		{
			// Stops the synchronised clock, disconnect the scenes, closes the server connection then stops the underlying transports.

			auto disconnectionTce = _disconnectionTce;
			auto loggerPtr = logger();
			auto serverConnectionWeak = _serverConnection;

			disconnectAllScenes()
				.then([serverConnectionWeak, disconnectionTce, loggerPtr](pplx::task<void> t)
			{
				try
				{
					t.get();
					auto serverConnection = serverConnectionWeak.lock();
					if (serverConnection)
					{
						serverConnection->close();
					}
					disconnectionTce.set();
				}
				catch (const std::exception& ex)
				{
					loggerPtr->log(LogLevel::Trace, "Client", "Ignore client disconnection failure", ex.what());
				}
			});
		}

		return pplx::create_task(_disconnectionTce);
	}

	pplx::task<void> Client::disconnect(Scene_ptr scene)
	{
		if (!scene)
		{
			throw std::runtime_error("Scene deleted");
		}

		if (scene->getCurrentConnectionState() != ConnectionState::Connected)
		{
			return pplx::task_from_exception<void>(std::runtime_error("The scene is not in connected state"));
		}

		auto sceneId = scene->id();
		auto sceneHandle = scene->handle();

		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			_scenes.erase(sceneId);
		}

		scene->setConnectionState(ConnectionState::Disconnecting);

		for (auto plugin : _plugins)
		{
			plugin->sceneDisconnecting(scene.get());
		}

		_dependencyResolver->resolve<SceneDispatcher>()->removeScene(sceneHandle);

		pplx::cancellation_token_source cts;

		auto timeOutToken = timeout(std::chrono::milliseconds(5000));

		auto loggerPtr = logger();

		return sendSystemRequest<void>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle)
			.then(createSafeCaptureNoThrow(STRM_WEAK_FROM_THIS(), [this, scene, sceneId]()
		{
			for (auto plugin : _plugins)
			{
				plugin->sceneDisconnected(scene.get());
			}

			logger()->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);

			scene->setConnectionState(ConnectionState::Disconnected);
		}), timeOutToken);
	}

	pplx::task<void> Client::disconnectAllScenes()
	{
		std::vector<pplx::task<void>> tasks;
		auto loggerPtr = logger();

		auto scenesToDisconnect = _scenes;
		std::lock_guard<std::mutex> lg(_scenesMutex);
		auto weakSelf = STRM_WEAK_FROM_THIS();
		auto self = weakSelf.lock();
		if (!self)
		{
			return pplx::task_from_result();
		}
		for (auto it : scenesToDisconnect)
		{
			tasks.push_back(it.second.task
				.then([weakSelf](Scene_ptr scene)
			{
				auto self = weakSelf.lock();
				if (self)
				{
					return self->disconnect(scene);
				}
				else
				{
					return pplx::task_from_result();
				}
			})
				.then([loggerPtr](pplx::task<void> t)
			{
				try
				{
					t.wait();
				}
				catch (const PointerDeletedException&)
				{
					// Swallow weak_from_this deleted exception
				}
				catch (const std::exception& ex)
				{
					loggerPtr->log(LogLevel::Error, "Client", "Scene disconnection failed", ex.what());
				}
			}));
		}

		return pplx::when_all(tasks.begin(), tasks.end());
	}

	void Client::transport_packetReceived(Packet_ptr packet)
	{
		for (auto plugin : _plugins)
		{
			plugin->packetReceived(packet);
		}

		_dependencyResolver->resolve<IPacketDispatcher>()->dispatchPacket(packet);
	}

	int64 Client::clock() const
	{
		return _dependencyResolver->resolve<SyncClock>()->clock();
	}

	int64 Client::lastPing() const
	{
		return _dependencyResolver->resolve<SyncClock>()->lastPing();
	}

	std::weak_ptr<DependencyResolver> Client::dependencyResolver()
	{
		return _dependencyResolver;
	}

	rxcpp::observable<ConnectionState> Client::getConnectionStateChangedObservable() const
	{
		return _connectionStateObservable.get_observable();
	}

	ConnectionState Client::getConnectionState() const
	{
		return _connectionState;
	}

	void Client::setMedatata(const std::string& key, const std::string& value)
	{
		_metadata[key] = value;
	}


	void Client::dispatchEvent(const std::function<void(void)>& ev)
	{
		auto actionDispatcher = _dependencyResolver->resolve<IActionDispatcher>();
		if (!actionDispatcher->isRunning())
		{
			logger()->log(LogLevel::Error, "Client", "Trying to post an event while dispatcher isn't running");
		}
		actionDispatcher->post(ev);
	}

	void Client::setConnectionState(ConnectionState state)
	{
		if (state != _connectionState )
		{
			_connectionState = state;
			

			// on_next and on_completed handlers could delete this. Do not use this after handlers have been called.
			auto actionDispatcher = _dependencyResolver->resolve<IActionDispatcher>();
			auto subscriber = _connectionStateObservable.get_subscriber();

			dispatchEvent([=]()
			{
				subscriber.on_next(state);
			
			});

			if (state == ConnectionState::Disconnected)
			{
				clear();
				actionDispatcher->stop(); // This task should never throw an exception, as it is triggered by a tce.
			}
		}
	}

	pplx::cancellation_token Client::getLinkedCancellationToken(pplx::cancellation_token ct)
	{
		return create_linked_source(ct, _cts.get_token(), Shutdown::instance().getShutdownToken()).get_token();
	}


















































	pplx::task<void> Client::ensureNetworkAvailable()
	{




































		return pplx::task_from_result();

	}
};
