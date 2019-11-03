#include "stormancer/stdafx.h"
#include "stormancer/Shutdown.h"
#include "stormancer/Client.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/ApiClient.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/TokenHandler.h"
#include "stormancer/SyncClock.h"
#include "stormancer/SceneInfosRequestDto.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/ConnectToSceneMsg.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/KeyStore.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/IPacketDispatcher.h"
#include "stormancer/Version.h"
















#include <cstring>

namespace
{













}

namespace Stormancer
{
	// Client

	Client::Client(Configuration_ptr config)
		: _initialized(false)
		, _accountId(config->account)
		, _applicationName(config->application)
		, _maxPeers(config->maxPeers)
		, _plugins(config->plugins())
		, _config(config)
		, _serverTimeout(config->defaultTimeout)
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

		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			for (const auto& scenePair : _scenes)
			{
				if (scenePair.second.task.is_done())
				{
					try
					{
						auto scene = scenePair.second.task.get();
						if (scene->getCurrentConnectionState() != ConnectionState::Disconnected)
						{
							scene->setConnectionState(ConnectionState::Disconnecting, false).get();
							scene->setConnectionState(ConnectionState::Disconnected, false).get();
						}
					}
					catch (...) {}
				}
			}
			_scenes.clear();
		}

		for (auto plugin : _plugins)
		{
			delete plugin;
		}
		_plugins.clear();
	}

	std::shared_ptr<IClient> IClient::create(Configuration_ptr config)
	{
		if (!config)
		{
			return nullptr;
		}

		std::shared_ptr<IClient> client = std::shared_ptr<Client>(new Client(config), [](Client* clientPtr) { delete clientPtr; });
		static_cast<Client*>(client.get())->initialize();
		return client;
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			ContainerBuilder builder;
			ConfigureContainer(builder, _config);
			for (auto plugin : _plugins)
			{
				plugin->registerClientDependencies(builder);
			}
			_dependencyResolver = builder.build();

			_dependencyResolver.resolve<IActionDispatcher>()->start();

			std::vector<std::shared_ptr<IRequestModule>> modules{ std::static_pointer_cast<IRequestModule>(_dependencyResolver.resolve<P2PRequestModule>()) };

			RequestProcessor::Initialize(_dependencyResolver.resolve<RequestProcessor>(), modules);

			auto transport = _dependencyResolver.resolve<ITransport>();
			auto wClient = STORM_WEAK_FROM_THIS();
			transport->onPacketReceived([wClient](Packet_ptr packet)
				{
					auto client = LockOrThrow(wClient);
					client->transport_packetReceived(packet);
				});

			logger()->log(LogLevel::Trace, "Client", "Starting transport", "port:" + std::to_string(_config->clientSDKPort) + "; maxPeers:" + std::to_string((uint16)_maxPeers + 1));

			transport->start("client", _dependencyResolver.resolve<IConnectionManager>(), _dependencyResolver, _cts.get_token(), _config->clientSDKPort, (uint16)_maxPeers + 1);
			for (auto plugin : _plugins)
			{
				plugin->transportStarted(transport);
			}
			_connections = _dependencyResolver.resolve<IConnectionManager>();
#if !defined(_WIN32)
			if (!_config->endpointRootCertificates.empty())
			{
				web::http::client::http_client_config::set_root_certificates(_config->endpointRootCertificates);
			}
#endif






























			for (auto plugin : _plugins)
			{
				plugin->clientCreated(this->shared_from_this());
			}

			std::string version(Version::getVersionString());
			logger()->log(LogLevel::Trace, "Client", "Initializing client (version: " + version + ")...");

			_metadata["serializers"] = "msgpack/array";
			_metadata["transport"] = transport->name();
			_metadata["version"] = version;



			_metadata["platform"] = __PLATFORM__;

			_metadata["protocol"] = "3";

			_disconnectionTce = pplx::task_completion_event<void>();
			_initialized = true;

			if (std::strcmp(_config->_headersVersion, Version::getVersionString()) != 0)
			{
				logger()->log(LogLevel::Warn, "Client", "The version of the stormancer headers (" + std::string(_config->_headersVersion) + ") is different from the version of the library (" + Version::getVersionString() + "), this may cause issues.");
			}
			logger()->log(LogLevel::Trace, "Client", "Client initialized");
		}
	}

	const std::string& Client::applicationName() const
	{
		return _applicationName;
	}

	ILogger_ptr Client::logger() const
	{
		auto result = _dependencyResolver.resolve<ILogger>();
		if (result)
		{
			return result;
		}
		else
		{
			return nullptr;
		}
	}

	pplx::task<std::shared_ptr<Scene>> Client::connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer, pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lg(this->_scenesMutex);

		std::weak_ptr<Client> wThat = this->shared_from_this();
		return parseSceneUrl(sceneId, ct).then([wThat, sceneId, initializer, ct](SceneAddress address) {
			auto that = wThat.lock();
			if (!that)
			{
				throw PointerDeletedException("Client destroyed");
			}

			return that->connectToScene(address, true, [wThat, sceneId, initializer, ct](const SceneAddress& address) {
				auto that = wThat.lock();
				if (!that)
				{
					throw PointerDeletedException("Client destroyed");
				}
				return that->getPublicScene(address, sceneId, ct)
					.then([initializer, ct, wThat](std::shared_ptr<Scene_Impl> scene)
						{

							if (initializer)
							{
								initializer(scene);
							}
							auto uSceneId = scene->address().toUri();


							return scene->connect(ct)
								.then([scene]()
									{
										return scene;
									}, ct);


						}, ct);
				});

			});

	}


	pplx::task<std::shared_ptr<Scene>> Client::connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer, pplx::cancellation_token ct)
	{


		Stormancer::SceneEndpoint sep;
		auto tokenHandler = _dependencyResolver.resolve<ITokenHandler>();
		if (sceneToken.substr(0, 1) == "{")
		{
			sep = tokenHandler->getSceneEndpointInfo(sceneToken);
		}
		else
		{
			sep = tokenHandler->decodeToken(sceneToken);

		}
		auto sceneId = sep.tokenData.SceneId;

		std::weak_ptr<Client> wThat = this->shared_from_this();
		return parseSceneUrl(sceneId, ct).then([wThat, sceneToken, initializer, ct](SceneAddress address) {
			auto that = wThat.lock();
			if (!that)
			{
				throw PointerDeletedException("Client destroyed");
			}

			return that->connectToScene(address, false, [wThat, sceneToken, initializer, ct](const SceneAddress& address) {
				auto that = wThat.lock();
				if (!that)
				{
					throw PointerDeletedException("Client destroyed");
				}
				return that->getPrivateScene(address, sceneToken, ct)
					.then([initializer, ct, wThat](std::shared_ptr<Scene_Impl> scene)
						{

							if (initializer)
							{
								initializer(scene);
							}
							auto uSceneId = scene->address().toUri();


							return scene->connect(ct)
								.then([scene]()
									{
										return scene;
									}, ct);


						}, ct);
				});

			});

	}
	pplx::task<std::shared_ptr<Scene>> Client::connectToScene(const SceneAddress& address, bool isPublic, const std::function<pplx::task<std::shared_ptr<Scene_Impl>>(const SceneAddress & address)> factory)
	{
		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		auto uri = address.toUri();
		auto it = _scenes.find(uri);
		if (it != _scenes.end())
		{
			return it->second.task.then([](std::shared_ptr<Scene_Impl> scene) {
				return std::static_pointer_cast<Scene>(scene);
				});
		}
		else
		{
			std::weak_ptr<Client> wThat = this->shared_from_this();
			ClientScene container;
			container.isPublic = isPublic;
			container.task = factory(address).then([wThat, uri](pplx::task<std::shared_ptr<Scene_Impl>> task) {
				if (auto that = wThat.lock())
				{
					try
					{
						auto scene = task.get();


						auto it = that->_scenes.find(uri);
						if (it != that->_scenes.end())
						{
							it->second.stateChangedSubscription = scene->getConnectionStateChangedObservable().subscribe([wThat, uri](ConnectionState state)
								{
									if (state == ConnectionState::Disconnecting)
									{
										if (auto that = wThat.lock())
										{
											that->_scenes.erase(uri);
										}
									}
									if (state == ConnectionState::Disconnected)
									{
										if (auto that = wThat.lock())
										{
											that->_scenes.erase(uri);
										}
									}

								});

						}
						return scene;
					}
					catch (std::exception & ex)
					{

						std::lock_guard<std::mutex> lg(that->_scenesMutex);
						that->_scenes.erase(uri);

						that->logger()->log(ex);
						throw;
					}
				}
				else
				{
					throw Stormancer::PointerDeletedException();
				}
				});
			_scenes.emplace(uri, container);
			return container.task.then([](std::shared_ptr<Scene_Impl> scene) {
				return std::static_pointer_cast<Scene>(scene);
				});
		}


	}

	pplx::task<std::shared_ptr<Scene>> Client::getConnectedScene(const std::string& sceneId, pplx::cancellation_token ct)
	{
		logger()->log(LogLevel::Trace, "Client", "Get connected scene.", sceneId);

		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "SceneId is empty");
			return pplx::task_from_exception<std::shared_ptr<Scene>>(std::runtime_error("SceneId is empty"));
		}

		std::lock_guard<std::mutex> lg(_scenesMutex);
		auto it = _scenes.find(sceneId);
		if (it != _scenes.end())
		{
			return it->second.task.then([](std::shared_ptr<Scene_Impl> scene) {
				return std::static_pointer_cast<Scene>(scene);
				});;

		}
		std::string scenelist;
		for (auto& s : _scenes)
		{
			scenelist += s.first;
			scenelist += ", ";
		}
		logger()->log(LogLevel::Trace, "Client", sceneId + " not found.", scenelist);
		return pplx::task_from_result<std::shared_ptr<Scene>>(nullptr);
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

	pplx::task<std::shared_ptr<Scene_Impl>> Client::getPublicScene(const SceneAddress& address, const std::string& sceneId, pplx::cancellation_token ct)
	{
		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Empty scene id.");
			return pplx::task_from_exception<std::shared_ptr<Scene_Impl>>(std::runtime_error("Empty scene id."));
		}

		std::weak_ptr<Client> wThat = this->shared_from_this();

		auto uSceneId = address.toUri();


		this->initialize();
		this->logger()->log(LogLevel::Trace, "Client", "Get public scene.", uSceneId);
		auto connections = this->_connections.lock();
		if (!connections)
		{
			throw PointerDeletedException("Client destroyed");
		}


		return ensureNetworkAvailable()
			.then([wThat, address, ct]()
				{
					auto client = LockOrThrow(wThat);
					return client->getFederation(ct);
				})
			.then([wThat, address, ct](Federation fed)
				{
					auto client = LockOrThrow(wThat);
					auto apiClient = client->_dependencyResolver.resolve<ApiClient>();
					return apiClient->getSceneEndpoint(fed.getCluster(address.clusterId).endpoints, address.account, address.app, address.sceneId, ct);
				}, ct)
					.then([wThat, address, ct](SceneEndpoint sep)
						{
							auto client = LockOrThrow(wThat);
							return client->getSceneInternal(address, sep, ct);
						}, ct);

	}

	pplx::task<std::shared_ptr<Scene_Impl>> Client::getPrivateScene(const SceneAddress& address, const std::string& sceneToken, pplx::cancellation_token ct)
	{

		this->initialize();
		logger()->log(LogLevel::Trace, "Client", "Get private scene.", address.toUri());

		if (sceneToken.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Empty scene token.");
			return pplx::task_from_exception<std::shared_ptr<Scene_Impl>>(std::runtime_error("Empty scene token."));
		}
		Stormancer::SceneEndpoint sep;
		auto tokenHandler = _dependencyResolver.resolve<ITokenHandler>();
		if (sceneToken.substr(0, 1) == "{")
		{
			sep = tokenHandler->getSceneEndpointInfo(sceneToken);
		}
		else
		{
			sep = tokenHandler->decodeToken(sceneToken);
		}
		auto sceneId = sep.tokenData.SceneId;

		std::weak_ptr<Client> wThat = this->shared_from_this();



		return getSceneInternal(address, sep, ct);


	}

	pplx::task<std::shared_ptr<IConnection>> Client::ensureConnectedToServer(std::string clusterId, SceneEndpoint sceneEndpoint, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);
		auto transport = _dependencyResolver.resolve<ITransport>();
		auto connections = _connections.lock();
		if (!connections || !transport)
		{
			throw PointerDeletedException("Client destroyed");
		}


		std::weak_ptr<Client> wClient = this->shared_from_this();
		return connections->getConnection(clusterId, [wClient, ct, sceneEndpoint](std::string id)
			{
				auto client = LockOrThrow(wClient);
				/*return client->getFederation(ct).then([wClient, ct, id](Federation fed) {
					auto cluster = fed.getCluster(id);
					auto client = LockOrThrow(wClient);
					auto api = client->dependencyResolver().resolve<ApiClient>();
					return api->GetServerEndpoints(cluster.endpoints);
				}).then([](ServerEndpoints endpoints) {
				});*/

				try
				{
					auto transport = client->dependencyResolver().resolve<ITransport>();
					//Start transport and execute plugin event

					//Protocol v1 is not supported anymore.
					//This client requires a grid with support for Federation/transport sniffing.

					//Connect to server
					std::srand((unsigned int)std::time(nullptr));
					auto it = sceneEndpoint.getTokenResponse.endpoints.find(transport->name());
					if (it == sceneEndpoint.getTokenResponse.endpoints.end())
					{
						throw std::runtime_error("No transport of type '" + transport->name() + "' available on this server");
					}
					auto& endpoints = it->second;
					auto endpointUrl = endpoints.at(std::rand() % endpoints.size());

					if (client->_config->forceTransportEndpoint != "")
					{
						endpointUrl = client->_config->forceTransportEndpoint;
					}
					client->logger()->log(LogLevel::Trace, "Client", "Connecting transport to server", endpointUrl);

					auto c = transport->connect(endpointUrl, id, "", ct)
						.then([wClient, sceneEndpoint, ct](std::shared_ptr<IConnection> connection)
							{
								auto client = LockOrThrow(wClient);

								connection->setMetadata(client->_metadata);
								connection->setMetadata("type", "server");

								if (client->_config->encryptionEnabled)
								{
									connection->setMetadata("encryption", sceneEndpoint.getTokenResponse.encryption.token);
									auto keyStore = client->_dependencyResolver.resolve<KeyStore>();
									auto key = utility::conversions::from_base64(utility::string_t(utility::conversions::to_string_t(sceneEndpoint.getTokenResponse.encryption.key)));
									if (key.size() != 256 / 8)
									{
										throw std::runtime_error(("Unexpected key size. received " + std::to_string(key.size() * 8) + " bits expected 256 bits ").c_str());
									}
									keyStore->keys.emplace(connection->sessionId(), key);
								}

								return connection->setTimeout(client->_serverTimeout, ct).then([connection]() { return connection; });
							}, ct)
						.then([ct](std::shared_ptr<IConnection> connection)
							{
								return connection->updatePeerMetadata(ct).then([connection]() { return connection; });
							}, ct)
								.then([wClient, ct](std::shared_ptr<IConnection> connection)
									{
										auto client = LockOrThrow(wClient);
										return client->requestSessionToken(connection, 1, ct).then([connection]() {return connection; });
									}, ct)
								.then([wClient, sceneEndpoint](std::shared_ptr<IConnection> connection)
									{
										auto client = LockOrThrow(wClient);
										if (client->_config->synchronisedClock)
										{
											client->_dependencyResolver.resolve<SyncClock>()->start(connection, client->_cts.get_token());
										}
										return connection;
									}, ct);
									return c;
				}
				catch (std::exception & ex)
				{
					auto reason = std::string("Failed to establish connection with ") + id + " : " + ex.what();

					throw std::runtime_error(reason.c_str());
				}
			});
	}

	pplx::task<std::shared_ptr<Scene_Impl>> Client::getSceneInternal(const SceneAddress& sceneAddress, const SceneEndpoint& sep, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		auto wClient = STORM_WEAK_FROM_THIS();

		return ensureConnectedToServer(sceneAddress.clusterId, sep, ct)
			.then([wClient, sep, ct](std::shared_ptr<IConnection> connection)
				{
					auto client = LockOrThrow(wClient);

					SceneInfosRequestDto parameter;

					if (!connection)
					{
						throw std::runtime_error("Connection not available");
					}
					parameter.Metadata = connection->metadata();
					parameter.Token = sep.token;

					client->logger()->log(LogLevel::Trace, "Client", "Send SceneInfosRequestDto");
					return client->sendSystemRequest<SceneInfosDto>(connection, (byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter, ct)
						.then([connection](SceneInfosDto sceneInfos)
							{
								return  std::make_tuple(sceneInfos, connection);
							});
				}, ct)
			.then([wClient, ct](std::tuple<SceneInfosDto, std::shared_ptr<IConnection>> tuple)
				{
					auto client = LockOrThrow(wClient);
					auto sceneInfos = std::get<0>(tuple);
					auto connection = std::get<1>(tuple);

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

					pplx::task<void> updateMetadataTask = pplx::task_from_result(pplx::task_options(ct));

					if (connection->trySetInitialSceneConnection())
					{
						connection->setMetadata("serializer", sceneInfos.SelectedSerializer);
						updateMetadataTask = connection->updatePeerMetadata(ct);
					}

					assert(!connection->metadata("serializer").empty());

					return updateMetadataTask.then([tuple]()
						{
							return tuple;
						}, ct);
				}, ct)
					.then([wClient, sceneAddress, sep](std::tuple<SceneInfosDto, std::shared_ptr<IConnection>> tuple)
						{
							auto client = LockOrThrow(wClient);
							auto sceneInfos = std::get<0>(tuple);
							auto connection = std::get<1>(tuple);

							std::shared_ptr<Scene_Impl> scene(new Scene_Impl(connection, wClient, sceneAddress, sep.token, sceneInfos, client->_dependencyResolver, client->_plugins), [](Scene_Impl* ptr) { delete ptr; });
							scene->initialize(client->_dependencyResolver);

							return scene;
						}, ct);
	}

	pplx::task<void> Client::connectToScene(std::shared_ptr<Scene_Impl> scene, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		auto wClient = STORM_WEAK_FROM_THIS();

		if (scene->getCurrentConnectionState() != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not in disconnected state.");
		}

		return scene->setConnectionState(ConnectionState::Connecting).then([scene, sceneToken, localRoutes, ct, wClient]
			{
				auto client = LockOrThrow(wClient);

				ConnectToSceneMessage parameter;
				parameter.Token = sceneToken;
				for (auto r : localRoutes)
				{
					RouteDto routeDto;
					routeDto.Handle = r->handle();
					routeDto.Name = r->name();
					routeDto.Metadata = r->metadata();
					parameter.Routes << routeDto;
				}
				auto connections = client->_connections.lock();
				if (!connections)
				{
					throw PointerDeletedException("Client destroyed");
				}

				auto serverConnection = scene->hostConnection().lock();

				if (!serverConnection)
				{
					throw PointerDeletedException("Connection not available");
				}
				parameter.ConnectionMetadata = serverConnection->metadata();
				parameter.SceneMetadata = scene->getSceneMetadata();

				return client->sendSystemRequest<ConnectionResult>(serverConnection, (byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter, ct);

			}, pplx::get_ambient_scheduler())
			.then([wClient, scene](ConnectionResult result)
				{
					auto client = LockOrThrow(wClient);
					scene->completeConnectionInitialization(result);
					auto sceneDispatcher = client->_dependencyResolver.resolve<SceneDispatcher>();
					auto serverConnection = scene->hostConnection().lock();
					sceneDispatcher->addScene(serverConnection, scene);

					client->logger()->log(LogLevel::Debug, "client", "Connected to scene", scene->address().toUri());

					return scene->setConnectionState(ConnectionState::Connected);
				}, ct);
	}

	pplx::task<void> Client::disconnect(pplx::cancellation_token ct)
	{
		if (_scenes.size() == 0)
		{
			return pplx::task_from_result(pplx::task_options(ct));
		}

		for (auto plugin : this->_plugins)
		{
			plugin->clientDisconnecting(this->shared_from_this());
		}

		auto loggerPtr = logger();
		auto wConnections = _connections;
		return disconnectAllScenes()
			.then([loggerPtr, wConnections, ct](pplx::task<void> t)
				{
					try
					{
						t.get();
					}
					catch (const std::exception & ex)
					{
						loggerPtr->log(LogLevel::Trace, "Client", "Ignore client disconnection failure", ex.what());
					}

					if (auto c = wConnections.lock())
					{
						return c->closeAllConnections("Client disconnection requested by user");
					}
					return pplx::task_from_result(pplx::task_options(ct));
				});
	}

	pplx::task<void> Client::disconnect(std::string sceneId, bool fromServer, std::string reason)
	{
		auto it = this->_scenes.find(sceneId);
		if (it == this->_scenes.end())
		{
			return pplx::task_from_result();
		}
		std::weak_ptr<Client> wThat = this->shared_from_this();
		return it->second.task
			.then([wThat, fromServer, reason](std::shared_ptr<Scene_Impl> scene)
				{
					if (auto that = wThat.lock())
					{
						that->disconnect(scene, fromServer, reason);
					}
				});
	}

	pplx::task<void> Client::disconnect(std::shared_ptr<IConnection> connection, uint8 sceneHandle, bool fromServer, std::string reason)
	{
		auto sceneDispatcher = _dependencyResolver.resolve<SceneDispatcher>();
		auto scene = sceneDispatcher->getScene(connection, sceneHandle);

		return disconnect(scene, fromServer, reason);
	}

	pplx::task<void> Client::disconnect(std::shared_ptr<Scene_Impl> scene, bool initiatedByServer, std::string reason, pplx::cancellation_token ct)
	{
		if (!scene)
		{
			return pplx::task_from_result(pplx::task_options(ct));
		}

		switch (scene->_connectionState)
		{
		case ConnectionState::Connected:
			// Do nothing, continue disconnection
			break;
		case ConnectionState::Disconnected:
			return pplx::task_from_result(pplx::task_options(ct));
			break;
		default:
			return pplx::task_from_exception<void>(std::runtime_error("The scene is not in connected state"), ct);
		}

		auto sceneId = scene->address().toUri();
		auto sceneHandle = scene->host()->handle();

		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			_scenes.erase(sceneId);
		}

		auto logger2 = logger();
		auto wClient = STORM_WEAK_FROM_THIS();
		return scene->setConnectionState(ConnectionState(ConnectionState::Disconnecting, reason))
			.then([wClient, sceneHandle, initiatedByServer, scene, ct]
				{
					auto client = wClient.lock();
					if (client)
					{
						auto connections = client->_connections.lock();
						if (connections)
						{
							if (auto c = connections->getConnection(scene->host()->id()))
							{
								auto sceneDispatcher = client->_dependencyResolver.resolve<SceneDispatcher>();
								sceneDispatcher->removeScene(c, sceneHandle);
								if (!initiatedByServer)
								{
									auto ct2 = create_linked_source(ct, timeout(std::chrono::seconds(5))).get_token();
									return client->sendSystemRequest<void>(c, (byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle, ct2);
								}
							}
						}
					}
					return pplx::task_from_result(pplx::task_options(ct));
				}, pplx::get_ambient_scheduler())
			.then([scene, sceneId, reason, logger2](pplx::task<void> task)
				{
					try
					{
						task.get();
					}
					catch (const std::exception & ex)
					{
						logger2->log(LogLevel::Trace, "client", "Error while disconnecting scene " + sceneId, ex.what());
					}
					logger2->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);
					return scene->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
				});
	}

	pplx::task<void> Client::disconnectAllScenes()
	{
		std::vector<pplx::task<void>> tasks;
		auto loggerPtr = logger();

		auto scenesToDisconnect = _scenes;
		std::lock_guard<std::mutex> lg(_scenesMutex);
		auto wClient = STORM_WEAK_FROM_THIS();
		auto client = wClient.lock();
		if (!client)
		{
			return pplx::task_from_result();
		}
		for (auto it : scenesToDisconnect)
		{
			tasks.push_back(it.second.task
				.then([wClient](std::shared_ptr<Scene_Impl> scene)
					{
						auto client = wClient.lock();
						if (client)
						{
							return client->disconnect(scene);
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
						catch (const std::exception & ex)
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

		if (_packetDispatcher.expired())
		{
			_packetDispatcher = _dependencyResolver.resolve<IPacketDispatcher>();
		}

		if (auto packetDispatcher = _packetDispatcher.lock())
		{
			packetDispatcher->dispatchPacket(packet);
		}
	}

	int64 Client::clock() const
	{
		return _dependencyResolver.resolve<SyncClock>()->clock();
	}

	int64 Client::lastPing() const
	{
		return _dependencyResolver.resolve<SyncClock>()->lastPing();
	}

	DependencyScope& Client::dependencyResolver()
	{
		return _dependencyResolver;
	}

	void Client::setMedatata(const std::string& key, const std::string& value)
	{
		_metadata[key] = value;
	}

	void Client::dispatchEvent(const std::function<void(void)>& ev)
	{
		auto actionDispatcher = _dependencyResolver.resolve<IActionDispatcher>();
		if (!actionDispatcher->isRunning())
		{
			logger()->log(LogLevel::Error, "Client", "Trying to post an event while dispatcher isn't running");
		}
		actionDispatcher->post(ev);
	}

	pplx::cancellation_token Client::getLinkedCancellationToken(pplx::cancellation_token ct)
	{
		return create_linked_source(ct, _cts.get_token(), Shutdown::instance().getShutdownToken()).get_token();
	}


















































	pplx::task<void> Client::ensureNetworkAvailable()
	{
























		return pplx::task_from_result();

	}

	pplx::task<void> Client::requestSessionToken(std::shared_ptr<IConnection> peer, int numRetries, pplx::cancellation_token ct)
	{
		auto wClient = STORM_WEAK_FROM_THIS();

		if (!peer)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Peer disconnected"));
		}
		auto requestProcessor = _dependencyResolver.resolve<RequestProcessor>();

		pplx::task<std::string> task;
		if (_sessionToken.empty())
		{
			task = requestProcessor->sendSystemRequest<std::string>(peer.get(), (byte)SystemRequestIDTypes::ID_JOIN_SESSION, ct);
		}
		else
		{
			task = requestProcessor->sendSystemRequest<std::string, std::string>(peer.get(), (byte)SystemRequestIDTypes::ID_JOIN_SESSION, _sessionToken, ct);
		}

		return task.then([wClient, peer, numRetries, ct](pplx::task<std::string> sessionTokenTask)
			{
				auto client = LockOrThrow(wClient);

				try
				{
					std::string serverToken = sessionTokenTask.get();
					if (client->_sessionToken.empty())
					{
						client->_sessionToken = serverToken;

					}

					return pplx::task_from_result(); // all control paths should return a value...
				}
				catch (const std::exception & ex)
				{
					client->_sessionToken.clear();
					if (numRetries > 0)
					{
						if (auto logger = client->logger())
						{
							logger->log(LogLevel::Warn, "Client::requestSessionToken", std::string("Server returned an error: ") + ex.what() + ", trying " + std::to_string(numRetries) + " more time(s)");
						}
						return client->requestSessionToken(peer, numRetries - 1, ct);
					}
					// No more retries: pass the exception to the caller
					throw ex;
				}
			}, ct);
	}

	pplx::task<Federation> Client::getFederation(pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lg(_connectionMutex);
		if (!_federation)
		{
			auto api = this->dependencyResolver().resolve<ApiClient>();
			_federation = std::make_shared<pplx::task<Federation>>(api->getFederation(_config->_serverEndpoints, ct));
		}
		return *_federation;
	}

	pplx::task<int> Client::pingCluster(std::string clusterId, pplx::cancellation_token ct)
	{
		std::weak_ptr<Client> wClient = this->shared_from_this();
		return getFederation(ct)
			.then([clusterId, ct, wClient](Federation fed)
				{
					auto client = LockOrThrow(wClient);
					auto api = client->dependencyResolver().resolve<ApiClient>();
					auto cluster = fed.getCluster(clusterId);
					return api->ping(cluster.endpoints.front(), ct);
				});
	}

	pplx::task<void> Client::setServerTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct)
	{
		_serverTimeout = timeout;

		if (auto connections = _connections.lock())
		{
			ct = create_linked_shutdown_token(ct);
			return connections->setTimeout(timeout, ct);
		}

		return pplx::task_from_result();
	}

	SceneAddress SceneAddress::parse(const std::string urn, const std::string& defaultClusterId, const std::string& defaultAccount, const std::string& defaultApp)
	{
		SceneAddress address;
		if (urn.find("scene:") != 0)
		{
			address.sceneId = urn;
		}
		else
		{
			std::vector<std::string> tokens;
			std::string token;
			std::istringstream tokenStream(urn);
			while (std::getline(tokenStream, token, '/'))
			{
				tokens.push_back(token);
			}
			address.sceneId = tokens[tokens.size() - 1];

			if (tokens.size() > 2)
			{
				address.app = tokens[tokens.size() - 2];
			}
			if (tokens.size() > 3)
			{
				address.account = tokens[tokens.size() - 3];
			}
			if (tokens.size() > 4)
			{
				address.clusterId = tokens[tokens.size() - 4];
			}
		}

		if (address.clusterId == "")
		{
			address.clusterId = defaultClusterId;
		}
		if (address.account == "")
		{
			address.account = defaultAccount;
		}
		if (address.app == "")
		{
			address.app = defaultApp;
		}
		return address;
	}

	pplx::task<SceneAddress> Client::parseSceneUrl(std::string url, pplx::cancellation_token ct)
	{
		auto config = this->_config;
		return getFederation(ct)
			.then([url, config](Federation fed)
				{
					return SceneAddress::parse(url, fed.current.id, config->account, config->application);
				});
	}
}
