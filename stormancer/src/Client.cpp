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
#include "stormancer/IConnectionManager.h"
#include "stormancer/IPacketDispatcher.h"
















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

	std::shared_ptr<IClient> IClient::create(Configuration_ptr config)
	{
		if (!config)
		{
			return nullptr;
		}

		Client_ptr client(new Client(config), [](Client* ptr) { delete ptr; });
		client->initialize();
		return std::reinterpret_pointer_cast<IClient>(client);
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

			logger()->log(LogLevel::Trace, "Client", "Starting transport", "port:" + std::to_string(_config->clientSDKPort) + "; maxPeers:" + std::to_string((uint16)_maxPeers + 1));

			transport->start("client", _dependencyResolver->resolve<IConnectionManager>(), _cts.get_token(), _config->clientSDKPort, (uint16)_maxPeers + 1);
			for (auto plugin : _plugins)
			{
				plugin->transportStarted(transport);
			}
			_connections = _dependencyResolver->resolve< IConnectionManager>();


































			for (auto plugin : _plugins)
			{
				plugin->clientCreated(this->shared_from_this());
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

	pplx::task<std::shared_ptr<Scene>> Client::connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer, pplx::cancellation_token ct)
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
				auto uSceneId = scene->address().normalize();
				scene->getConnectionStateChangedObservable().subscribe([wThat, uSceneId](ConnectionState state) {
					if (state == ConnectionState::Disconnecting)
						if (auto that = wThat.lock())
						{
							that->_scenes.erase(uSceneId);
						}
				});

				return scene->connect(ct)
					.then([scene]()
				{
					return scene;
				}, ct).then([wThat, uSceneId](pplx::task<Scene_ptr> t) {

					try
					{
						return t.get();
					}
					catch (std::exception&)
					{
						if (auto that = wThat.lock())
						{
							that->_scenes.erase(uSceneId);
						}
						throw;
					}
				});
			}
			else
			{
				return pplx::task_from_result(scene);
			}
		}, ct).then([](Scene_ptr s) {return std::reinterpret_pointer_cast<Scene>(s); });
	}

	pplx::task<std::shared_ptr<Scene>> Client::connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer, pplx::cancellation_token ct)
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
				auto uSceneId = scene->address().normalize();
				scene->getConnectionStateChangedObservable().subscribe([wThat, uSceneId](ConnectionState state) {
					if (state == ConnectionState::Disconnecting)
						if (auto that = wThat.lock())
						{
							std::lock_guard<std::mutex> lg(that->_scenesMutex);
							that->_scenes.erase(uSceneId);
						}
				});

				return scene->connect(ct)
					.then([scene]()
				{
					return scene;
				}, ct).then([wThat, uSceneId](pplx::task<Scene_ptr> t) {

					try
					{
						return t.get();
					}
					catch (std::exception&)
					{
						if (auto that = wThat.lock())
						{
							std::lock_guard<std::mutex> lg(that->_scenesMutex);
							that->_scenes.erase(uSceneId);
						}
						throw;
					}
				});
			}
			else
			{
				return pplx::task_from_result(scene);
			}


		}, ct).then([](Scene_ptr s) {return std::reinterpret_pointer_cast<Scene>(s); });
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
			return it->second.task.then([](Scene_ptr s) {return std::reinterpret_pointer_cast<Scene>(s); });
		}

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

	pplx::task<Scene_ptr> Client::getPublicScene(const std::string& sceneId, pplx::cancellation_token ct)
	{
		if (sceneId.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Empty scene id.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Empty scene id."));
		}

		std::weak_ptr<Client> wThat = this->shared_from_this();
		return parseSceneUrl(sceneId, ct).then([wThat, ct](SceneAddress address) {
			auto uSceneId = address.normalize();
			auto that = LockOrThrow(wThat);
			std::lock_guard<std::mutex> lg(that->_scenesMutex);
			that->initialize();
			that->logger()->log(LogLevel::Trace, "Client", "Get public scene.", uSceneId);
			auto connections = that->_connections.lock();
			if (!connections)
			{
				throw PointerDeletedException("Client destroyed");
			}



			auto it = that->_scenes.find(uSceneId);
			if (it != that->_scenes.end())
			{
				auto& container = it->second;
				if (container.isPublic)
				{
					return container.task;
				}
				else
				{
					that->logger()->log(LogLevel::Error, "Client", "The scene is private.");
					return pplx::task_from_exception<Scene_ptr>(std::runtime_error("The scene is private."));
				}
			}
			else
			{

				ClientScene cScene;
				cScene.isPublic = true;




				auto task = that->ensureNetworkAvailable()
					.then([wThat, address, ct]() {
					auto client = LockOrThrow(wThat);
					return client->getFederation(ct);
				})
					.then([wThat, address, ct](Federation fed)
				{
					auto client = LockOrThrow(wThat);
					auto apiClient = client->_dependencyResolver->resolve<ApiClient>();
					return apiClient->getSceneEndpoint(fed.getCluster(address.clusterId).endpoints, address.account, address.app, address.sceneId, ct);
				}, ct)
					.then([wThat, address, ct](SceneEndpoint sep)
				{
					auto client = LockOrThrow(wThat);
					return client->getSceneInternal(address, sep, ct);
				}, ct).then([wThat, address, uSceneId](pplx::task<Scene_ptr> t)
				{
					try
					{
						return t.get();
					}
					catch (std::exception&)
					{
						auto client = LockOrThrow(wThat);
						std::lock_guard<std::mutex> lg(client->_scenesMutex);
						client->_scenes.erase(uSceneId);
						throw;
					}
				});

				cScene.task = task;
				that->_scenes.emplace(uSceneId, cScene);
				return task;
			}
		});
	}

	pplx::task<Scene_ptr> Client::getPrivateScene(const std::string& sceneToken, pplx::cancellation_token ct)
	{


		std::lock_guard<std::mutex> lg(this->_scenesMutex);
		this->initialize();
		logger()->log(LogLevel::Trace, "Client", "Get private scene.");

		if (sceneToken.empty())
		{
			logger()->log(LogLevel::Error, "Client", "Empty scene token.");
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Empty scene token."));
		}

		auto tokenHandler = _dependencyResolver->resolve<ITokenHandler>();
		auto sep = tokenHandler->decodeToken(sceneToken);
		auto sceneId = sep.tokenData.SceneId;

		std::weak_ptr<Client> wThat = this->shared_from_this();
		return parseSceneUrl(sceneId, ct).then([wThat, ct, sep](SceneAddress sceneAddress) {

			auto that = LockOrThrow(wThat);
			auto uSceneId = sceneAddress.normalize();
			auto it = that->_scenes.find(uSceneId);
			if (it != that->_scenes.end())
			{
				auto& container = it->second;

				return container.task;

			}
			else
			{

				ClientScene cScene;
				cScene.isPublic = true;
				auto task = that->getSceneInternal(sceneAddress, sep, ct).then([wThat, uSceneId](pplx::task<Scene_ptr> t) {
					try
					{
						return t.get();
					}
					catch (...)
					{
						if (auto that = wThat.lock())
						{
							that->_scenes.erase(uSceneId);
						}
						throw;
					}
				});

				cScene.task = task;
				that->_scenes.emplace(uSceneId, cScene);
				return task;
			}
		});
	}

	pplx::task<std::shared_ptr<IConnection>> Client::ensureConnectedToServer(std::string clusterId, SceneEndpoint sceneEndpoint, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);
		auto transport = _dependencyResolver->resolve<ITransport>();
		auto connections = _connections.lock();
		if (!connections || !transport)
		{
			throw PointerDeletedException("Client destroyed");
		}

		std::weak_ptr<Client> wClient = this->shared_from_this();
		return connections->getConnection(clusterId, [wClient, ct, sceneEndpoint](std::string id) {
			auto client = LockOrThrow(wClient);
			/*return client->getFederation(ct).then([wClient, ct, id](Federation fed) {
				auto cluster = fed.getCluster(id);
				auto client = LockOrThrow(wClient);
				auto api = client->dependencyResolver().lock()->resolve<ApiClient>();
				return api->GetServerEndpoints(cluster.endpoints);
			}).then([](ServerEndpoints endpoints) {


			});*/

			try
			{

				auto transport = client->dependencyResolver()->resolve<ITransport>();
				//Start transport and execute plugin event

				//Protocol v1 is not supported anymore.
				//This client requires a grid with support for Federation/transport sniffing.

				//Connect to server

				auto& endpoints = sceneEndpoint.getTokenResponse.endpoints.at(transport->name());
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
						auto keyStore = client->_dependencyResolver->resolve<KeyStore>();
						auto key = utility::conversions::from_base64(utility::string_t(sceneEndpoint.getTokenResponse.encryption.key.begin(), sceneEndpoint.getTokenResponse.encryption.key.end()));
						if (key.size() != 256 / 8)
						{
							throw std::runtime_error(("Unexpected key size. received " + std::to_string(key.size() * 8) + " bits expected 256 bits ").c_str());
						}
						keyStore->keys.emplace(connection->id(), key);

					}

					return client->updateServerMetadata(connection, ct).then([connection]() {return connection; });

				}, ct)
					.then([wClient, ct](std::shared_ptr<IConnection> connection)
				{
					auto client = LockOrThrow(wClient);
					return client->createOrJoinSession(connection, ct).then([connection]() {return connection; });
				}, ct)
					.then([wClient, sceneEndpoint](std::shared_ptr<IConnection> connection)
				{
					auto client = LockOrThrow(wClient);
					if (client->_config->synchronisedClock)
					{
						client->_dependencyResolver->resolve<SyncClock>()->start(connection, client->_cts.get_token());
					}
					return connection;
				}, ct);
				return c;
			}
			catch (std::exception& ex)
			{
				auto reason = std::string("Failed to establish connection with ") + id + " : " + ex.what();

				throw std::runtime_error(reason.c_str());
			}

		});


	}

	pplx::task<Scene_ptr> Client::getSceneInternal(const SceneAddress& sceneAddress, const SceneEndpoint& sep, pplx::cancellation_token ct)
	{
		ct = getLinkedCancellationToken(ct);

		auto wClient = STRM_WEAK_FROM_THIS();

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
			return client->sendSystemRequest<SceneInfosDto>(connection, (byte)SystemRequestIDTypes::ID_GET_SCENE_INFOS, parameter, ct).then([connection](SceneInfosDto sceneInfos) {
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


			connection->setMetadata("serializer", sceneInfos.SelectedSerializer);

			return client->updateServerMetadata(connection, ct)
				.then([tuple]()
			{
				return tuple;
			}, ct);
		}, ct)
			.then([wClient, sceneAddress, sep](std::tuple<SceneInfosDto, std::shared_ptr<IConnection>> tuple)
		{
			auto client = LockOrThrow(wClient);
			auto sceneInfos = std::get<0>(tuple);
			auto connection = std::get<1>(tuple);


			Scene_ptr scene(new Scene_Impl(connection, wClient, sceneAddress, sep.token, sceneInfos, client->_dependencyResolver, client->_plugins), [](Scene_Impl* ptr) { delete ptr; });
			scene->initialize();


			return scene;
		}, ct);
	}

	pplx::task<void> Client::updateServerMetadata(std::shared_ptr<IConnection> serverConnection, pplx::cancellation_token ct)
	{
		auto loggerPtr = logger();
		loggerPtr->log(LogLevel::Trace, "Client", "Update server metadata");

		ct = getLinkedCancellationToken(ct);
		auto connections = _connections.lock();
		if (!connections)
		{
			return pplx::task_from_exception<void>(Stormancer::PointerDeletedException(""));
		}


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
		auto connections = _connections.lock();
		if (!connections)
		{
			throw PointerDeletedException("Client destroyed");
		}

		auto serverConnection = connections->getConnection(scene->host()->id());


		if (!serverConnection)
		{
			throw PointerDeletedException("Connection not available");
		}
		parameter.ConnectionMetadata = serverConnection->metadata();

		return client->sendSystemRequest<ConnectionResult>(serverConnection, (byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, parameter, ct)
			.then([wClient, scene, serverConnection](ConnectionResult result)
		{
			auto client = LockOrThrow(wClient);
			scene->completeConnectionInitialization(result);
			auto sceneDispatcher = client->_dependencyResolver->resolve<SceneDispatcher>();
			sceneDispatcher->addScene(serverConnection, scene);
			scene->setConnectionState(ConnectionState::Connected);

			client->logger()->log(LogLevel::Debug, "client", "Scene connected.", scene->id());

		}, ct);
	}

	pplx::task<void> Client::disconnect()
	{
		if (_scenes.size() == 0)
		{
			return pplx::task_from_result();
		}

		for (auto plugin : this->_plugins)
		{
			plugin->clientDisconnecting(this->shared_from_this());
		}

		// Stops the synchronised clock, disconnect the scenes, closes the server connection then stops the underlying transports.

		auto loggerPtr = logger();

		auto wConnections = _connections;
		return disconnectAllScenes()
			.then([loggerPtr, wConnections](pplx::task<void> t)
		{
			try
			{
				t.get();
				if (auto c = wConnections.lock())
				{
					return c->closeAllConnections("Client disconnection requested by user");
				}


			}
			catch (const std::exception& ex)
			{
				loggerPtr->log(LogLevel::Trace, "Client", "Ignore client disconnection failure", ex.what());
			}
			return pplx::task_from_result();
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
		return it->second.task.then([wThat, fromServer, reason](Scene_ptr scene) {
			if (auto that = wThat.lock())
			{
				that->disconnect(scene, fromServer, reason);
			}
		});
	}

	pplx::task<void> Client::disconnect(Scene_ptr scene, bool initiatedByServer, std::string reason)
	{
		if (!scene)
		{
			throw std::runtime_error("Scene deleted");
		}

		if (scene->getCurrentConnectionState() != ConnectionState::Connected)
		{
			return pplx::task_from_exception<void>(std::runtime_error("The scene is not in connected state"));
		}

		auto sceneId = scene->address().normalize();
		auto sceneHandle = scene->handle();

		{
			std::lock_guard<std::mutex> lg(_scenesMutex);
			_scenes.erase(sceneId);
		}

		scene->setConnectionState(ConnectionState(ConnectionState::Disconnecting, reason));

		auto sceneDispatcher = _dependencyResolver->resolve<SceneDispatcher>();
		auto connections = _connections.lock();

		if (!sceneDispatcher || !connections)
		{
			throw PointerDeletedException("Client destroyed");
		}

		auto c = connections->getConnection(scene->host()->id());
		sceneDispatcher->removeScene(c, sceneHandle);


		pplx::cancellation_token_source cts;

		auto timeOutToken = timeout(std::chrono::milliseconds(5000));
		if (!initiatedByServer)
		{
			auto wClient = STRM_WEAK_FROM_THIS();
			return sendSystemRequest<void>(c, (byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle)
				.then([wClient, scene, sceneId, reason]()
			{
				if (auto client = wClient.lock())
				{
					

					client->logger()->log(LogLevel::Debug, "client", "Scene disconnected", sceneId);

					scene->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
				}
			}, timeOutToken);
		}
		else
		{


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

	std::shared_ptr<DependencyResolver> Client::dependencyResolver()
	{
		return _dependencyResolver;
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



	pplx::cancellation_token Client::getLinkedCancellationToken(pplx::cancellation_token ct)
	{
		return create_linked_source(ct, _cts.get_token(), Shutdown::instance().getShutdownToken()).get_token();
	}


















































	pplx::task<void> Client::ensureNetworkAvailable()
	{




































		return pplx::task_from_result();

	}

	pplx::task<void> Client::createOrJoinSession(std::shared_ptr<IConnection> peer, pplx::cancellation_token ct)
	{
		auto wClient = STRM_WEAK_FROM_THIS();


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

	pplx::task<Federation> Client::getFederation(pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lg(_connectionMutex);
		auto api = this->dependencyResolver()->resolve<ApiClient>();
		if (!_federation)
		{
			_federation = std::make_shared<pplx::task<Federation>>(api->getFederation(_config->_serverEndpoints, ct));
		}
		return *_federation;
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
		return getFederation(ct).then([url, config](Federation fed) {
			return SceneAddress::parse(url, fed.current.id, config->account, config->application);

		});
	}
};
