#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Configuration.h"
#include "stormancer/SceneEndpoint.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/SceneDispatcher.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/IConnection.h"
#include "stormancer/IScheduler.h"
#include "stormancer/Watch.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/IClient.h"

namespace Stormancer
{
	class Client;
	class IConnectionManager;
	struct Federation;
	class P2PRequestModule;
	using Client_ptr = std::shared_ptr<Client>;

	struct getSceneResult
	{
		bool created;
		pplx::task<std::shared_ptr<Scene_Impl>> scene;
	};
	/// Manage the connection to the scenes of an application.
	class STORMANCER_DLL_API Client : public IClient, public std::enable_shared_from_this<Client>
	{
		friend P2PRequestModule;

	public:

		/// The scene need to access some private methods of the client.
		friend class Scene_Impl;
		friend class IClient;

#pragma region public_methods

		pplx::task<std::shared_ptr<Scene>> connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer = SceneInitializer(), pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		pplx::task<std::shared_ptr<Scene>> connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer = SceneInitializer(), pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		pplx::task<std::shared_ptr<Scene>> getConnectedScene(const std::string& sceneId, pplx::cancellation_token ct = pplx::cancellation_token::none());

		std::vector<std::string> getSceneIds();

		/// Returns the name of the application.
		const std::string& applicationName() const;

		/// Returns the used logger.
		ILogger_ptr logger() const;

		/// Disconnect and close all connections.
		/// this method returns nothing. So it's useful for application close.
		pplx::task<void> disconnect(pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		/// Get sync clock value
		int64 clock() const override;

		/// Get last ping from sync clock
		int64 lastPing() const override;

		/// Get dependency resolver
		DependencyScope& dependencyResolver() override;

		/// Set a metadata
		void setMedatata(const std::string& key, const std::string& value) override;

		pplx::task<Federation> getFederation(pplx::cancellation_token ct) override;

		pplx::task<int> pingCluster(std::string clusterId, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		pplx::task<void> setServerTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct) override;

#pragma endregion

	private:

#pragma region private_classes

		struct ClientScene
		{
			pplx::task<std::shared_ptr<Scene_Impl>> task;
			bool connecting = false;
			bool isPublic = true;
			rxcpp::subscription stateChangedSubscription;

			virtual ~ClientScene()
			{
				if (stateChangedSubscription.is_subscribed())
				{
					stateChangedSubscription.unsubscribe();
				}
			}
		};

#pragma endregion

#pragma region private_methods

		Client(Configuration_ptr config);
		Client(const Client& other) = delete;
		Client(const Client&& other) = delete;
		Client& operator=(const Client& other) = delete;
		~Client();
		void initialize();
		pplx::task<std::shared_ptr<Scene_Impl>> getPublicScene(const SceneAddress& address, const std::string& sceneId, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<std::shared_ptr<Scene_Impl>> getPrivateScene(const SceneAddress& address, const std::string& sceneToken, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> connectToScene(std::shared_ptr<Scene_Impl> sceneId, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> disconnect(std::string sceneId, bool fromServer, std::string reason = "");
		pplx::task<void> disconnect(std::shared_ptr<IConnection> connection, uint8 sceneHandle, bool fromServer, std::string reason = "");
		pplx::task<void> disconnect(std::shared_ptr<Scene_Impl> scene, bool fromServer = false, std::string reason = "", pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> disconnectAllScenes();
		pplx::task<std::shared_ptr<Scene_Impl>> getSceneInternal(const SceneAddress& sceneId, const SceneEndpoint& sceneEndpoint, pplx::cancellation_token ct = pplx::cancellation_token::none());
		void transport_packetReceived(Packet_ptr packet);
		pplx::task<std::shared_ptr<IConnection>> ensureConnectedToServer(std::string clusterId, SceneEndpoint sceneEndpoint, pplx::cancellation_token ct = pplx::cancellation_token::none());
		void dispatchEvent(const std::function<void(void)>& ev);

		pplx::cancellation_token getLinkedCancellationToken(pplx::cancellation_token ct);
		void ConfigureContainer(ContainerBuilder& builder, Configuration_ptr config);
		// Request a session token from the server, or set it if we already have one (i.e in case of a reconnection)
		// In case of failure, clear the session token and retry, at most numRetries times.
		pplx::task<void> requestSessionToken(std::shared_ptr<IConnection> connection, int numRetries = 1, pplx::cancellation_token ct = pplx::cancellation_token::none());

		pplx::task<SceneAddress> parseSceneUrl(std::string url, pplx::cancellation_token ct);

		pplx::task<std::shared_ptr<Scene>> connectToScene(const SceneAddress& address, bool isPublic, std::function<pplx::task<std::shared_ptr<Scene_Impl>>(const SceneAddress& address)> factory);




		pplx::task<void> ensureNetworkAvailable();

		/// Sends a system request to the remote peer.
		/// \param connection Peer connection.
		/// \param id System request ID (SystemRequestIDTypes).
		/// \param T2 Parameters to send to the remote peer.
		/// \param ct Cancellation token (default to none).
		template<typename T1, typename T2>
		pplx::task<T1> sendSystemRequest(std::shared_ptr<IConnection> connection, byte id, const T2& parameter, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			auto requestProcessor = dependencyResolver().resolve<RequestProcessor>();
			return requestProcessor->sendSystemRequest<T1, T2>(connection.get(), id, parameter, ct);
		}

#pragma endregion

#pragma region private_members

		DependencyScope _dependencyResolver;

		pplx::task<void> _currentTask = pplx::task_from_result();
		bool _initialized = false;
		std::string _accountId;
		std::string _applicationName;

		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		std::unordered_map<std::string, std::string> _metadata;
		double _offset = 0;
		std::unordered_map<std::string, ClientScene> _scenes;
		std::mutex _scenesMutex;
		std::vector<IPlugin*> _plugins;
		std::mutex _connectionMutex;
		pplx::task_completion_event<void> _disconnectionTce;
		bool _connectionTaskSet = false;
		Action<ConnectionState> _onConnectionStateChanged;
		std::shared_ptr<Configuration> _config;
		rxcpp::composite_subscription _connectionSubscription;
		std::weak_ptr<IPacketDispatcher> _packetDispatcher;
		Serializer _serializer;
		std::string _sessionToken;

		bool firstInit = true;

		std::shared_ptr<pplx::task<Federation>> _federation;
		std::weak_ptr<IConnectionManager> _connections;
		std::chrono::milliseconds _serverTimeout;

#pragma endregion
	};
}
