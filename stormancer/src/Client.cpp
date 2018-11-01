#include "stormancer/stdafx.h"
#include "stormancer/Shutdown.h"
#include "stormancer/Client.h"
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

	Client::Client(Configuration_ptr config) :
		_initialized(false)
		, _accountId(config->account)
		, _applicationName(config->application)
		, _maxPeers(config->maxPeers)
		, _metadata(config->_metadata)
		, _plugins(config->plugins())
		, _config(config)
		, _dependencyResolver(std::make_shared<DependencyResolver>())
	{
	}

	Client::~Client()
	{
		logger()->log(LogLevel::Trace, "Client", "Deleting the client...");

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
		client->initialize();
		return client;
	}

	void Client::initialize()
	{
		if (firstInit)
		{
			firstInit = false;
			ConfigureContainer(_dependencyResolver, _config);
			std::vector<std::shared_ptr<IRequestModule>> modules{ std::dynamic_pointer_cast<IRequestModule>(_dependencyResolver->resolve<P2PRequestModule>()) };

			RequestProcessor::Initialize(_dependencyResolver->resolve<RequestProcessor>(), modules);

			auto transport = _dependencyResolver->resolve<ITransport>();
			auto wClient = STRM_WEAK_FROM_THIS();
			transport->onPacketReceived([wClient](Packet_ptr packet)
			{
				auto client = LockOrThrow(wClient);
				client->transport_packetReceived(packet);
			});



































			for (auto plugin : _plugins)
			{
				plugin->clientCreated(this);
			}
		}

		if (!_initialized)
		{


			logger()->log(LogLevel::Trace, "Client", "Creating the client...");



			logger()->log(LogLevel::Trace, "Client", "Client created");

			logger()->log(LogLevel::Trace, "Client", "Initializing client...");
			auto transport = _dependencyResolver->resolve<ITransport>();

			_metadata["serializers"] = "msgpack/array";
			_metadata["transport"] = transport->name();
			_metadata["version"] = "1.3.0";



			_metadata["platform"] = __PLATFORM__;

			_metadata["protocol"] = "2";


			auto actionDispatcher = _dependencyResolver->resolve<IActionDispatcher>();
			actionDispatcher->start();
			_disconnectionTce = pplx::task_completion_event<void>();
			_initialized = true;
			_dependencyResolver->resolve<IActionDispatcher>()->start();

			_cts = pplx::cancellation_token_source();
			logger()->log(LogLevel::Trace, "Client", "Client initialized");
		}
	}

	void Client::clear()
	{
		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		_initialized = false;
		_metadata.clear();
		//_dependencyResolver = nullptr;
		this->_initialized = false;


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
		std::weak_ptr<Client> wThat = this->shared_from_this();
		return getPublicScene(sceneId, ct)
			.then([initializer, ct, wThat, sceneId](Scene_ptr scene)
		{
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}
				scene->getConnectionStateChangedObservable().subscribe([wThat, sceneId](ConnectionState state) {
					if (state == ConnectionState::Disconnecting)
						if (auto that = wThat.lock())
						{
							that->_scenes.erase(sceneId);
						}
				});

				return scene->connect(ct)
					.then([scene]()
				{
					return scene;
				}, ct).then([wThat, sceneId](pplx::task<Scene_ptr> t) {

					try
					{
						return t.get();
					}
					catch (std::exception&)
					{
						if (auto that = wThat.lock())
						{
							that->_scenes.erase(sceneId);
						}
						throw;
					}
				});
			}
			else
			{
				return pplx::task_from_result(scene);
			}
		}, ct);
	}

	pplx::task<Scene_ptr> Client::connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer, pplx::cancellation_token ct)
	{
		auto tokenHandler = _dependencyResolver->resolve<ITokenHandler>();
		auto sep = tokenHandler->decodeToken(sceneToken);
		auto sceneId = sep.tokenData.SceneId;

		std::weak_ptr<Client> wThat = this->shared_from_this();

		return getPrivateScene(sceneToken, ct)
			.then([initializer, ct, wThat, sceneId](Scene_ptr scene)
		{
			if (scene->getCurrentConnectionState() == ConnectionState::Disconnected)
			{
				if (initializer)
				{
					initializer(scene);
				}

				scene->getConnectionStateChangedObservable().subscribe([wThat, sceneId](ConnectionState state) {
					if (state == ConnectionState::Disconnecting)
						if (auto that = wThat.lock())
						{
							std::lock_guard<std::mutex> lg(that->_scenesMutex);
							that->_scenes.erase(sceneId);
						}
				});

				return scene->connect(ct)
					.then([scene]()
				{
					return scene;
				}, ct).then([wThat, sceneId](pplx::task<Scene_ptr> t) {

					try
					{
						return t.get();
					}
					catch (std::exception&)
					{
						if (auto that = wThat.lock())
						{
							std::lock_guard<std::mutex> lg(that->_scenesMutex);
							that->_scenes.erase(sceneId);
						}
						throw;
					}
				});
			}
			else
			{
				return pplx::task_from_result(scene);
			}


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
		auto it = _scenes.find(sceneId);
		if (it != _scenes.end())
		{
			return it->second.task;
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

		auto it = _scenes.find(sceneId);
		if (it != _scenes.end())
		{
			auto& container = it->second;
			if (container.isPublic)
			{
				return container.task;
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

			auto wClient = STRM_WEAK_FROM_THIS();

			auto task = ensureNetworkAvailable()
				.then([wClient, sceneId, ct]()
			{
				auto client = LockOrThrow(wClient);
				auto apiClient = client->_dependencyResolver->resolve<ApiClient>();
				return apiClient->getSceneEndpoint(client->_accountId, client->_applicationName, sceneId, ct);
			}, ct)
				.then([wClient, sceneId, ct](SceneEndpoint sep)
			{
				auto client = LockOrThrow(wClient);
				return client->getSceneInternal(sceneId, sep, ct);
			}, ct).then([wClient, sceneId](pplx::task<Scene_ptr> t)
			{
				try
				{
					return t.get();
				}
				catch (std::exception&)
				{
					auto client = LockOrThrow(wClient);
					std::lock_guard<std::mutex> lg(client->_scenesMutex);
					client->_scenes.erase(sceneId);
					throw;
				}
			});

			cScene.task = task;
			_scenes.emplace(sceneId, cScene);
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
								throw std::runtime_error(("Unexpected key size. received " + std::to_string(key.size() * 8) + " bits expected 256 bits ").c_str());
							}
							std::copy(key.begin(), key.end(), keyStore->key);
						}
					}

					if (this->_config->forceTransportEndpoint != "")
					{
						endpointUrl = this->_config->forceTransportEndpoint;
					}
					logger()->log(LogLevel::Trace, "Client", "Connecting transport to server", endpointUrl);
					auto wClient = STRM_WEAK_FROM_THIS();
					_connectionTask = transport->connect(endpointUrl, ct)
						.then([wClient](std::weak_ptr<IConnection> connectionWeak)
					{
						auto client = LockOrThrow(wClient);

						auto connection = connectionWeak.lock();
						if (!connection)
						{
							throw std::runtime_error("Connection not available");
						}

						connection->setMetadata("type", "server");

						auto onConnectionStateChangedNext = [wClient](ConnectionState state) {
							if (auto client = wClient.lock())
							{
								if (state == ConnectionState::Disconnecting || state == ConnectionState::Disconnected)
								{
									std::lock_guard<std::mutex> lg(client->_scenesMutex);
									for (auto sceneIt : client->_scenes)
									{
										sceneIt.second.task
											.then([wClient, state](Scene_ptr scene)
										{
											if (auto client = wClient.lock())
											{
												auto sceneState = scene->getCurrentConnectionState();

												// We check the connection is disconnecting, and the scene is not already disconnecting or disconnected
												if (state == ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnected)
												{
													// We are disconnecting the scene
													client->dispatchEvent([scene, state]()
													{
														scene->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
													});

													// We notify the scene is disconnecting to the plugin
													for (auto plugin : client->_plugins)
													{
														plugin->sceneDisconnecting(scene.get());
													}
												}
												// We check the connection is disconnected, and the scene is not already disconnected
												else if (state == ConnectionState::Disconnected && sceneState != ConnectionState::Disconnected)
												{
													// We ensure the scene is disconnecting
													if (sceneState != ConnectionState::Disconnecting)
													{
														client->dispatchEvent([scene, state]()
														{
															scene->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
														});
													}

													// We disconnect the scene
													client->dispatchEvent([scene, state]()
													{
														scene->setConnectionState(ConnectionState(ConnectionState::Disconnected, state.reason));
													});

													// We notify the scene is disconnected to the plugin
													for (auto plugin : client->_plugins)
													{
														plugin->sceneDisconnected(scene.get());
													}
												}
											}
										})
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

									if (state == ConnectionState::Disconnected)
									{
										//client->_cts.cancel(); // this will cause the syncClock and the transport to stop
										client->_serverConnection.reset();
										client->_connectionSubscription.unsubscribe();
									}
								}

								client->setConnectionState(state);
							}
						};

						auto loggerPtr = client->logger();
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

						client->_connectionSubscription = connection->getConnectionStateChangedObservable().subscribe(onConnectionStateChangedNext, onConnectionStateChangedError);

						client->setConnectionState(connection->getConnectionState());

						// Flatten all external addresses into a comma-separated string
						/*std::stringstream addrs;
						auto transport = _dependencyResolver->resolve<ITransport>();
						for (const std::string& addr : transport->externalAddresses())
						{
							addrs << addr << ",";
						}
						_metadata["externalAddrs"] = addrs.str();
						_metadata["externalAddrs"].pop_back();*/

						connection->setMetadata(client->_metadata);
						client->_serverConnection = connectionWeak;
						return;
					}, ct)
						.then([wClient, ct]()
					{
						auto client = LockOrThrow(wClient);
						return client->updateServerMetadata(ct);
					}, ct)
						.then([wClient, ct]()
					{
						auto client = LockOrThrow(wClient);
						return client->requestSessionToken();
					}, ct)
						.then([wClient, endpoint]()
					{
						auto client = LockOrThrow(wClient);
						if (client->_config->synchronisedClock)
						{
							client->_dependencyResolver->resolve<SyncClock>()->start(client->_serverConnection, client->_cts.get_token());
						}
					}, ct)
						.then([wClient](pplx::task<void> task)
					{
						try
						{
							task.get();
						}
						catch (const std::exception& ex)
						{
							if (auto client = wClient.lock())
							{
								auto reason = ex.what();
								if (auto connection = client->_serverConnection.lock())
								{
									connection->close(reason);
								}
								client->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
							}
							throw;
						}
						catch (...) // Catching all exceptions (including managed exceptions (C++/CLI))
						{
							if (auto client = wClient.lock())
							{
								auto reason = "Connection failed";
								if (auto connection = client->_serverConnection.lock())
								{
									connection->close(reason);
								}
								client->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
							}
							throw;
						}
					}); // no ct here because we need this continuation to awalys run
				}
				catch (std::exception& ex)
				{
					auto reason = std::string("Failed to start transport: ") + ex.what();
					setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
					return pplx::task_from_exception<void>(std::runtime_error(reason.c_str()));
				}
			}
		}

		return _connectionTask;
	}

	pplx::task<Scene_ptr> Client::getSceneInternal(const std::string& sceneId, const SceneEndpoint& sep, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		logger()->log(LogLevel::Trace, "Client", "Get scene " + sceneId, sep.token);

		auto wClient = STRM_WEAK_FROM_THIS();

		return ensureConnectedToServer(sep, ct)
			.then([wClient, sep, ct]()
		{
			auto client = LockOrThrow(wClient);

			SceneInfosRequestDto parameter;
			auto serverConnection = client->_serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("Connection not available");
			}
			parameter.Metadata = serverConnection->metadata();
			parameter.Token = sep.token;

			client->logger()->log(LogLevel::Trace, "Client", "Send SceneInfosRequestDto");
			return client->sendSystemRequest<SceneInfosDto>((byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter, ct);
		}, ct)
			.then([wClient, ct](SceneInfosDto sceneInfos)
		{
			auto client = LockOrThrow(wClient);

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

			client->logger()->log(LogLevel::Trace, "Client", "SceneInfosDto received", ss.str());

			auto serverConnection = client->_serverConnection.lock();
			if (!serverConnection)
			{
				throw std::runtime_error("Connection not available");
			}
			serverConnection->setMetadata("serializer", sceneInfos.SelectedSerializer);

			return client->updateServerMetadata(ct)
				.then([sceneInfos]()
			{
				return sceneInfos;
			}, ct);
		}, ct)
			.then([wClient, sceneId, sep](SceneInfosDto sceneInfos)
		{
			auto client = LockOrThrow(wClient);

			client->logger()->log(LogLevel::Trace, "Client", "Return the scene", sceneId);
			Scene_ptr scene(new Scene(client->_serverConnection, wClient, sceneId, sep.token, sceneInfos, client->_dependencyResolver), [](Scene* ptr) { delete ptr; });
			scene->initialize();
			for (auto plugin : client->_plugins)
			{
				plugin->registerSceneDependencies(scene.get());
			}
			for (auto plugin : client->_plugins)
			{
				plugin->sceneCreated(scene.get());
			}
			return scene;
		}, ct);
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

		auto serializer = _serializer;
		return requestProcessor->sendSystemRequest(serverConnection.get(), (byte)SystemRequestIDTypes::ID_SET_METADATA, [serializer, serverConnection](obytestream* stream)
		{
			serializer.serialize(stream, serverConnection->metadata());
		}, PacketPriority::MEDIUM_PRIORITY, ct)
			.then([loggerPtr](Packet_ptr)
		{
			loggerPtr->log(LogLevel::Trace, "Client", "Updated server metadata");
		}, ct);
	}

	pplx::task<void> Client::connectToScene(const Scene_ptr& scene, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		std::lock_guard<std::mutex> lg(_scenesMutex);

		auto wClient = STRM_WEAK_FROM_THIS();

		auto client = LockOrThrow(wClient);

		if (scene->getCurrentConnectionState() != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not in disconnected state.");
		}

		scene->setConnectionState(ConnectionState::Connecting);

		for (auto plugin : client->_plugins)
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

		auto serverConnection = client->_serverConnection.lock();
		if (!serverConnection)
		{
			throw std::runtime_error("Connection not available");
		}
		parameter.ConnectionMetadata = serverConnection->metadata();

		return client->sendSystemRequest<ConnectionResult>((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter, ct)
			.then([wClient, scene](ConnectionResult result)
		{
			auto client = LockOrThrow(wClient);
			scene->completeConnectionInitialization(result);
			auto sceneDispatcher = client->_dependencyResolver->resolve<SceneDispatcher>();
			sceneDispatcher->addScene(scene);
			scene->setConnectionState(ConnectionState::Connected);
			for (auto plugin : client->_plugins)
			{
				plugin->sceneConnected(scene.get());
			}
			client->logger()->log(LogLevel::Debug, "client", "Scene connected.", scene->id());

		}, ct);
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
			for (auto plugin : this->_plugins)
			{
				plugin->clientDisconnecting(this);
			}

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
					if (auto serverConnection = serverConnectionWeak.lock())
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

	pplx::task<void> Client::disconnect(std::string sceneId, bool fromServer, std::string reason)
	{
		auto it = this->_scenes.find(sceneId);
		if (it == this->_scenes.end())
		{
			return pplx::task_from_result();
		}
		std::weak_ptr<Client> wThat = this->shared_from_this();
		return it->second.task.then([wThat, fromServer, reason](Scene_ptr scene) {
			if (auto that = wThat.lock())
			{
				that->disconnect(scene, fromServer, reason);
			}
		});
	}

	pplx::task<void> Client::disconnect(Scene_ptr scene, bool fromServer, std::string reason)
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

		scene->setConnectionState(ConnectionState(ConnectionState::Disconnecting, reason));

		for (auto plugin : _plugins)
		{
			try
			{
				plugin->sceneDisconnecting(scene.get());
			}
			catch (const std::exception& ex)
			{
				auto logger = this->logger();
				if (logger)
				{
					logger->log(Stormancer::LogLevel::Warn, "client.disconnect", "a plugin threw an error while running sceneDisconnecting", ex.what());
					logger->log(ex);
				}
			}
		}

		auto sceneDispatcher = _dependencyResolver->resolve<SceneDispatcher>();
		if (sceneDispatcher)
		{
			sceneDispatcher->removeScene(sceneHandle);
		}

		pplx::cancellation_token_source cts;

		auto timeOutToken = timeout(std::chrono::milliseconds(5000));
		if (!fromServer)
		{
			auto wClient = STRM_WEAK_FROM_THIS();
			return sendSystemRequest<void>((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle)
				.then([wClient, scene, sceneId, reason]()
			{
				if (auto client = wClient.lock())
				{
					for (auto plugin : client->_plugins)
					{
						plugin->sceneDisconnected(scene.get());
					}

					client->logger()->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);

					scene->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
				}
			}, timeOutToken);
		}
		else
		{
			for (auto plugin : this->_plugins)
			{
				plugin->sceneDisconnected(scene.get());
			}

			this->logger()->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);

			scene->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
			return pplx::task_from_result();
		}
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
		if (state != _connectionState)
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

	pplx::task<void> Client::requestSessionToken(pplx::cancellation_token ct)
	{
		auto wClient = STRM_WEAK_FROM_THIS();

		auto peer = _serverConnection.lock();
		if (!peer)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Peer disconnected"));
		}
		auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();

		pplx::task<std::string> task;
		if (_sessionToken.empty())
		{
			task = requestProcessor->sendSystemRequest<std::string>(peer.get(), (byte)SystemRequestIDTypes::ID_JOIN_SESSION, ct);
		}
		else
		{
			task = requestProcessor->sendSystemRequest<std::string, std::string>(peer.get(), (byte)SystemRequestIDTypes::ID_JOIN_SESSION, _sessionToken, ct);
		}
		
		return task.then([wClient](std::string sessionToken)
		{
			auto client = LockOrThrow(wClient);
			client->_sessionToken = sessionToken;
		}, ct);
	}
};
