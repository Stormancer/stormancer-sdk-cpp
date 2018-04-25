#include "stormancer/stdafx.h"
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
















namespace
{













}

namespace Stormancer
{
	// Client

	Client::Client(Configuration_ptr config)
		: _dependencyResolver(std::make_shared<DependencyResolver>())
		, _initialized(false)
		, _accountId(config->account)
		, _applicationName(config->application)
		, _maxPeers(config->maxPeers)
		, _metadata(config->_metadata)
		, _synchronisedClock(config->synchronisedClock)
		, _plugins(config->plugins())
		, _config(config)
	{
		ConfigureContainer(this->dependencyResolver(), config);

		logger()->log(LogLevel::Trace, "Client", "Creating the client...");

		std::vector<std::shared_ptr<IRequestModule>> modules{ std::dynamic_pointer_cast<IRequestModule>(dependencyResolver()->resolve<P2PRequestModule>()) };

		RequestProcessor::Initialize(this->dependencyResolver()->resolve<RequestProcessor>(), modules);


































		logger()->log(LogLevel::Trace, "Client", "Client created");
	}

	pplx::task<void> Client::destroy()
	{
		auto logger = _dependencyResolver->resolve<ILogger>();

		logger->log(LogLevel::Trace, "Client", "Deleting client...");

		auto disconnectTask = disconnect().then([logger](pplx::task<void> t) {
			try
			{
				t.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Trace, "Client", "Ignore client disconnection failure", ex.what());
			}
		});

		_cts.cancel();

		return disconnectTask;
	}

	void Client::clean()
	{
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

		_dependencyResolver = nullptr;
		_metadata.clear();
		_config = nullptr;
		_connectionSubscription.clear();
		_onConnectionStateChanged.clear();
	}

	Client::~Client()
	{
		clean();
	}

	Client_ptr Client::create(Configuration_ptr config)
	{
		if (!config)
		{
			return nullptr;
		}

		auto result = std::shared_ptr<Client>(new Client(config), [](Client* client)
		{
			auto logger = client->logger();
			client->destroy().then([logger, client](pplx::task<void> t) {
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Warn, "Client", "Client destroy failed", ex.what());
				}
				logger->log(LogLevel::Trace, "Client", "Running shared_ptr finalizer.");
				client->clean();
				delete client;
			});
		});
		result->_weak = result;
		result->initialize();
		return result;
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
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

			_initialized = true;
			_dependencyResolver->resolve<IActionDispatcher>()->start();
			transport->onPacketReceived(createSafeCapture(weak_from_this(), [this](Packet_ptr packet) {
				transport_packetReceived(packet);
			}));
			logger()->log(LogLevel::Trace, "Client", "Client initialized");
		}
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
		logger()->log(LogLevel::Trace, "Client", "Get public scene.", sceneId);

		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Bad scene id.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Bad scene id."));
		}

		std::lock_guard<std::mutex> lg(_scenesMutex);
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
				.then(createSafeCapture(weak_from_this(), [this, sceneId, ct]()
			{
				auto apiClient = _dependencyResolver->resolve<ApiClient>();
				return apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, ct);
			}), ct)
				.then(createSafeCapture(weak_from_this(), [this, sceneId, ct](SceneEndpoint sep)
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
		logger()->log(LogLevel::Trace, "Client", "Get private scene.", sceneToken);

		if (sceneToken.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Bad scene token.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Bad scene token."));
		}

		auto tokenHandler = _dependencyResolver->resolve<ITokenHandler>();
		auto sep = tokenHandler->decodeToken(sceneToken);
		auto sceneId = sep.tokenData.SceneId;

		std::lock_guard<std::mutex> lg(_scenesMutex);
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
					logger()->log(LogLevel::Trace, "Client", "Starting transport", "port:" + std::to_string(_config->serverPort) + "; maxPeers:" + std::to_string((uint16)_maxPeers + 1));
					auto transport = _dependencyResolver->resolve<ITransport>();
					transport->start("client", _dependencyResolver->resolve<IConnectionManager>(), _cts.get_token(), _config->serverPort, (uint16)_maxPeers + 1);
					for (auto plugin : _plugins)
					{
						plugin->transportStarted(transport.get());
					}

					//Connect to server
					std::string endpointUrl = endpoint.tokenData.Endpoints.at(transport->name());
					logger()->log(LogLevel::Trace, "Client", "Connecting transport to server", endpointUrl);
					_connectionTask = transport->connect(endpointUrl, ct)
						.then(createSafeCapture(weak_from_this(), [this](std::weak_ptr<IConnection> connectionWeak)
					{
						auto connection = connectionWeak.lock();
						if (!connection)
						{
							throw std::runtime_error("Connection not available");
						}

						auto onConnectionStateChangedNext = createSafeCaptureNoThrow(weak_from_this(), [this](ConnectionState state) {
							if (state == ConnectionState::Disconnecting)
							{
								std::lock_guard<std::mutex> lg(_scenesMutex);
								for (auto sceneIt : _scenes)
								{
									sceneIt.second.task
										.then(createSafeCapture(weak_from_this(), [this](Scene_ptr scene)
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
										.then(createSafeCapture(weak_from_this(), [this](Scene_ptr scene)
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
						.then(createSafeCapture(weak_from_this(), [this, ct]()
					{
						return updateServerMetadata(ct);
					}), ct)
						.then(createSafeCapture(weak_from_this(), [this, endpoint]()
					{
						if (_synchronisedClock && endpoint.tokenData.Version > 0)
						{
							_dependencyResolver->resolve<SyncClock>()->start(_serverConnection, _cts.get_token());
						}
					}), ct);
				}
				catch (std::exception& ex)
				{
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
			.then(createSafeCapture(weak_from_this(), [this, sep, ct]()
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
			.then(createSafeCapture(weak_from_this(), [this, ct](SceneInfosDto sceneInfos)
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
			.then(createSafeCapture(weak_from_this(), [this, sceneId, sep](SceneInfosDto sceneInfos)
		{
			logger()->log(LogLevel::Trace, "Client", "Return the scene", sceneId);
			auto scene = std::make_shared<Scene>(_serverConnection.lock().get(), weak_from_this(), sceneId, sep.token, sceneInfos, _dependencyResolver);
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

		return requestProcessor->sendSystemRequest(serverConnection.get(), (byte)SystemRequestIDTypes::ID_SET_METADATA, createSafeCapture(weak_from_this(), [this, serverConnection](obytestream* stream) {
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
			.then(createSafeCapture(weak_from_this(), [this, sceneToken, localRoutes, ct](Scene_ptr scene)
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
				.then(createSafeCapture(weak_from_this(), [this, scene](ConnectionResult result)
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

		auto timeOutToken = timeout(std::chrono::milliseconds(30));

		auto loggerPtr = logger();

		return sendSystemRequest<void>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle)
			.then(createSafeCaptureNoThrow(weak_from_this(), [this, scene, sceneId]()
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

		std::lock_guard<std::mutex> lg(_scenesMutex);

		auto scenesToDisconnect = _scenes;
		for (auto it : scenesToDisconnect)
		{
			tasks.push_back(it.second.task
				.then(createSafeCapture(weak_from_this(), [this](Scene_ptr scene)
			{
				return disconnect(scene);
			}))
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

	DependencyResolver* Client::dependencyResolver()
	{
		return _dependencyResolver.get();
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
		if (state != _connectionState && !_connectionStateObservableCompleted)
		{
			_connectionState = state;
			_connectionStateObservableCompleted = (state == ConnectionState::Disconnected);

			// on_next and on_completed handlers could delete this. Do not use this after handlers have been called.
			auto actionDispatcher = _dependencyResolver->resolve<IActionDispatcher>();
			auto subscriber = _connectionStateObservable.get_subscriber();

			dispatchEvent([=]()
			{
				subscriber.on_next(state);
				if (state == ConnectionState::Disconnected)
				{
					subscriber.on_completed();
				}
			});

			if (state == ConnectionState::Disconnected)
			{
				actionDispatcher->stop(); // This task should never throw an exception, as it is triggered by a tce.
			}
		}
	}

	pplx::cancellation_token Client::getLinkedCancellationToken(pplx::cancellation_token ct)
	{
		return create_linked_source(ct, _cts.get_token()).get_token();
	}

	std::weak_ptr<Client> Client::weak_from_this()
	{
		return _weak;
	}


















































	pplx::task<void> Client::ensureNetworkAvailable()
	{




































		return pplx::task_from_result();

	}
};
