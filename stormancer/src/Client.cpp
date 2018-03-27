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

#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Creating the client...");
#endif


		std::vector<std::shared_ptr<IRequestModule>> modules{ std::dynamic_pointer_cast<IRequestModule>(dependencyResolver()->resolve<P2PRequestModule>()) };

		RequestProcessor::Initialize(this->dependencyResolver()->resolve<RequestProcessor>(), modules);


























#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Client created");
#endif
	}

	pplx::task<void> Client::destroy()
	{
		auto logger = _dependencyResolver->resolve<ILogger>();

#ifdef STORMANCER_LOG_CLIETN
		logger->log(LogLevel::Trace, "Client", "Deleting client...");
#endif

		_cts.cancel();

		return disconnect().then([=](pplx::task<void> t) {
			try
			{
				t.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Warn, "Client", "Client disconnection failed", ex.what());
			}
		});
	}

	Client::~Client()
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
	}

	Client_ptr Client::create(Configuration_ptr config)
	{
		if (!config)
		{
			return nullptr;
		}

		auto client = std::shared_ptr<Client>(new Client(config), [](Client* client)
		{
			auto logger = client->logger();
			client->destroy().then([=](pplx::task<void> t) {
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Warn, "Client", "Client destroy failed", ex.what());
				}
				delete client;
			});
		});
		client->initialize();
		return client;
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
#ifdef STORMANCER_LOG_CLIENT
			logger()->log(LogLevel::Trace, "Client", "Initializing client...");
#endif
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
			transport->onPacketReceived([=](Packet_ptr packet) { transport_packetReceived(packet); });
#ifdef STORMANCER_LOG_CLIENT
			logger()->log(LogLevel::Trace, "Client", "Client initialized");
#endif
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

	pplx::task<Scene_ptr> Client::connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer)
	{
		pplx::task<Scene_ptr> task = getPublicScene(sceneId);

		return task.then([=](Scene_ptr scene) {
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}
			}
			return scene->connect().then([=]() {
				return scene;
			});
		});
	}

	pplx::task<Scene_ptr> Client::connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer)
	{
		pplx::task<Scene_ptr> task = getPrivateScene(sceneToken);

		return task.then([=](Scene_ptr scene) {
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}
			}
			return scene->connect().then([=]() {
				return scene;
			});
		});
	}

	STORMANCER_DLL_API pplx::task<Scene_ptr> Client::getConnectedScene(const std::string& sceneId)
	{
#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Get connected scene.", sceneId);
#endif

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

	pplx::task<Scene_ptr> Client::getPublicScene(const std::string& sceneId)
	{
#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Get public scene.", sceneId);
#endif

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
				.then([=]() {
				
				return _dependencyResolver->resolve<ApiClient>()->getSceneEndpoint(_accountId, _applicationName, sceneId).then([=](SceneEndpoint sep) {
				return getSceneInternal(sceneId, sep);
				});
			});
			cScene.task = task;
			_scenes[sceneId] = cScene;
			return task;
		}
	}

	pplx::task<Scene_ptr> Client::getPrivateScene(const std::string& sceneToken)
	{
#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Get private scene.", sceneToken);
#endif

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
			auto task = getSceneInternal(sep.tokenData.SceneId, sep);
			cScene.task = task;
			_scenes[sceneId] = cScene;
			return task;
		}
	}

	pplx::task<void> Client::ensureConnectedToServer(const SceneEndpoint& endpoint)
	{
		if (_connectionState == ConnectionState::Disconnected)
		{
			std::lock_guard<std::mutex> lock(_connectionMutex);
			if (_connectionState == ConnectionState::Disconnected)
			{
				setConnectionState(ConnectionState::Connecting);

				try
				{
					//Start transport and execute plugin event
#ifdef STORMANCER_LOG_CLIENT
					logger()->log(LogLevel::Trace, "Client", "Starting transport", "port:" + std::to_string(_config->serverPort) + "; maxPeers:" + std::to_string((uint16)_maxPeers + 1));
#endif
					auto transport = _dependencyResolver->resolve<ITransport>();
					transport->start("client", _dependencyResolver->resolve<IConnectionManager>(), _cts.get_token(), _config->serverPort, (uint16)_maxPeers + 1);
					for (auto plugin : _plugins)
					{
						plugin->transportStarted(transport.get());
					}

					//Connect to server
					std::string endpointUrl = endpoint.tokenData.Endpoints.at(transport->name());
#ifdef STORMANCER_LOG_CLIENT
					logger()->log(LogLevel::Trace, "Client", "Connecting transport to server", endpointUrl);
#endif
					_connectionTask =  transport->connect(endpointUrl)
						.then([=](std::weak_ptr<IConnection> weakptr)
					{
						auto connection = weakptr.lock();
						if (!connection)
						{
							return;
						}

						_connectionSubscription = connection->getConnectionStateChangedObservable().subscribe([=](ConnectionState state)
						{
							// On next
							if (state == ConnectionState::Disconnecting)
							{
								std::lock_guard<std::mutex> lg(_scenesMutex);
								for (auto sceneIt : _scenes)
								{
									sceneIt.second.task.then([=](Scene_ptr scene) {
										dispatchEvent([=]() {
											scene->setConnectionState(ConnectionState::Disconnecting);
										});
										for (auto plugin : _plugins)
										{
											plugin->sceneDisconnecting(scene.get());
										}
									}).then([](pplx::task<void> t)
									{
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
							}
							else if (state == ConnectionState::Disconnected)
							{
								std::lock_guard<std::mutex> lg(_scenesMutex);
								for (auto sceneIt : _scenes)
								{
									sceneIt.second.task.then([=](Scene_ptr scene) {
										dispatchEvent([=]() {
											scene->setConnectionState(ConnectionState::Disconnected);
										});
										for (auto plugin : _plugins)
										{
											plugin->sceneDisconnected(scene.get());
										}
									}).then([](pplx::task<void> t)
									{
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
						}, [=](std::exception_ptr exptr) {
							// On error
							try
							{
								std::rethrow_exception(exptr);
							}
							catch (const std::exception& ex)
							{
								logger()->log(LogLevel::Error, "Client", "Connection state change failed", ex.what());
							}
						});

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
						_serverConnection = weakptr;
						return;
					})
						.then([=]()
					{
						return updateServerMetadata();
					})
						.then([=]()
					{
						if (_synchronisedClock && endpoint.tokenData.Version > 0)
						{
							_dependencyResolver->resolve<SyncClock>()->start(_serverConnection, _cts.get_token());
						}
					});
				}
				catch (std::exception& ex)
				{
					return pplx::task_from_exception<void>(std::runtime_error(std::string("Failed to start transport: ") + ex.what()));
				}
			}
		}

		return _connectionTask;
	}

	pplx::task<Scene_ptr> Client::getSceneInternal(const std::string& sceneId, const SceneEndpoint& sep)
	{
#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Get scene " + sceneId, sep.token);
#endif

		return ensureConnectedToServer(sep).then([=]() {
			SceneInfosRequestDto parameter;
			auto serverConnection = _serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("The server connection ptr is invalid");
			}
			parameter.Metadata = serverConnection->metadata();
			parameter.Token = sep.token;

#ifdef STORMANCER_LOG_CLIENT
			logger()->log(LogLevel::Trace, "Client", "Send SceneInfosRequestDto");
#endif
			return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter);
		}).then([=](SceneInfosDto sceneInfos) {
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
				throw std::runtime_error("The server connection ptr is invalid");
			}
			serverConnection->setMetadata("serializer", sceneInfos.SelectedSerializer);

			return updateServerMetadata().then([=]() {
				return sceneInfos;
			});
		}).then([=](SceneInfosDto sceneInfos) {
			logger()->log(LogLevel::Trace, "Client", "Return the scene", sceneId);
			auto scene = std::make_shared<Scene>(_serverConnection.lock().get(), shared_from_this(), sceneId, sep.token, sceneInfos, _dependencyResolver);
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
		});
	}

	pplx::task<void> Client::updateServerMetadata()
	{
#ifdef STORMANCER_LOG_CLIENT
		logger()->log(LogLevel::Trace, "Client", "Update server metadata");
#endif

		auto serverConnection = _serverConnection.lock();
		if (!serverConnection)
		{
			throw std::runtime_error("The server connection ptr is invalid");
		}

		auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();
		return requestProcessor->sendSystemRequest(serverConnection.get(), (byte)SystemRequestIDTypes::ID_SET_METADATA, [=](obytestream* stream) {
			_serializer.serialize(stream, serverConnection->metadata());
		}).then([this](Packet_ptr) {
#ifdef STORMANCER_LOG_CLIENT
			logger()->log(LogLevel::Trace, "Client", "Updated server metadata");
#endif
		});
	}

	pplx::task<void> Client::connectToScene(const std::string& sceneId, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes)
	{
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

		auto& task = cScene.task;

		task = task.then([=](Scene_ptr scene) {
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
				throw std::runtime_error("The server connection pointer is invalid.");
			}
			parameter.ConnectionMetadata = serverConnection->metadata();

			auto systemRequestTask = sendSystemRequest<ConnectToSceneMsg, ConnectionResult>((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter);

			return systemRequestTask.then([=](ConnectionResult result) {
				scene->completeConnectionInitialization(result);
				auto sceneDispatcher = _dependencyResolver->resolve<SceneDispatcher>();
				sceneDispatcher->addScene(scene);
				scene->setConnectionState(ConnectionState::Connected);
				for (auto plugin : _plugins)
				{
					plugin->sceneConnected(scene.get());
				}
#ifdef STORMANCER_LOG_CLIENT
				logger()->log(LogLevel::Debug, "client", "Scene connected.", scene->id());
#endif
				return scene;
			});
		});

		cScene.connecting = true;

		return task.then([](Scene_ptr) {});
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

			auto connection = _serverConnection;
			disconnectAllScenes().then([=](pplx::task<void> t) {
				try
				{
					t.get();
					auto serverConnection = connection.lock();
					if (serverConnection)
					{
						serverConnection->close();
					}
					_disconnectionTce.set();
				}
				catch (const std::exception& ex)
				{
					logger()->log(LogLevel::Warn, "Client", "Exception thrown in client disconnection", ex.what());
				}
			});
		}

		return pplx::create_task(_disconnectionTce);
	}

	pplx::task<void> Client::disconnect(Scene_ptr scene)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene pointer is null");
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

		return sendSystemRequest<byte, void>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle).then([=](pplx::task<void> t) {
			try
			{
				t.get();
				for (auto plugin : _plugins)
				{
					plugin->sceneDisconnected(scene.get());
				}

#ifdef STORMANCER_LOG_CLIENT			
				logger()->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);
#endif

				scene->setConnectionState(ConnectionState::Disconnected);
			}
			catch (const std::exception& ex)
			{
				// capture the exception and don't rethrow it
				logger()->log(LogLevel::Warn, "client", "Scene disconnexion failed", ex.what());
			}
			catch (...)
			{
				// capture the exception and don't rethrow it
				logger()->log(LogLevel::Warn, "client", "Scene disconnexion failed");
			}
		});
	}

	pplx::task<void> Client::disconnectAllScenes()
	{
		std::vector<pplx::task<void>> tasks;

		std::lock_guard<std::mutex> lg(_scenesMutex);
		auto scenesToDisconnect = _scenes;
		for (auto it : scenesToDisconnect)
		{
			tasks.push_back(it.second.task.then([=](Scene_ptr scene) {
				return disconnect(scene);
			}));
		}

		return 	pplx::when_all(tasks.begin(), tasks.end());
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
#ifdef STORMANCER_LOG_CLIENT
		if (!actionDispatcher->isRunning())
		{
			logger()->log(LogLevel::Error, "Client", "Trying to post an event while dispatcher isn't running");
		}
#endif
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


















































	pplx::task<void> Client::ensureNetworkAvailable()
	{



































		return pplx::task_from_result();

	}

};
