#pragma once

#include "stormancer/headers.h"
#include "stormancer/Configuration.h"
#include "stormancer/SceneEndpoint.h"
#include "stormancer/Scene.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/SceneDispatcher.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/IConnection.h"
#include "stormancer/IScheduler.h"
#include "stormancer/Watch.h"
#include "stormancer/ITokenHandler.h"

namespace Stormancer
{
	class Client;
	using Client_ptr = std::shared_ptr<Client>;

	/// Manage the connection to the scenes of an application.
	class Client : public std::enable_shared_from_this<Client>
	{
	public:

		/// The scene need to access some private methods of the client.
		friend class Scene;

		using SceneInitializer = std::function<void(Scene_ptr)>;

#pragma region public_methods

		/// Factory.
		/// \param config A pointer to a Configuration.
		STORMANCER_DLL_API static Client_ptr create(Configuration_ptr config);

		STORMANCER_DLL_API pplx::task<Scene_ptr> getPublicScene(const std::string& sceneId);

		STORMANCER_DLL_API pplx::task<Scene_ptr> getPrivateScene(const std::string& sceneToken);

		STORMANCER_DLL_API pplx::task<Scene_ptr> connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer = SceneInitializer());

		STORMANCER_DLL_API pplx::task<Scene_ptr> connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer = SceneInitializer());

		STORMANCER_DLL_API pplx::task<Scene_ptr> getConnectedScene(const std::string& sceneId);

		STORMANCER_DLL_API std::vector<std::string> getSceneIds();

		/// Returns the name of the application.
		STORMANCER_DLL_API const std::string& applicationName() const;

		/// Returns the used logger.
		STORMANCER_DLL_API ILogger_ptr logger() const;

		/// Disconnect and close all connections.
		/// this method returns nothing. So it's useful for application close.
		STORMANCER_DLL_API pplx::task<void> disconnect();

		/// Get sync clock value
		STORMANCER_DLL_API int64 clock() const;

		/// Get last ping from sync clock
		STORMANCER_DLL_API int64 lastPing() const;

		/// Get dependency resolver
		STORMANCER_DLL_API DependencyResolver* dependencyResolver();

		/// Get connection state observable
		STORMANCER_DLL_API rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const;

		/// Get current connection state
		STORMANCER_DLL_API ConnectionState getConnectionState() const;

		/// Set a metadata
		STORMANCER_DLL_API void setMedatata(const std::string& key, const std::string& value);


#pragma endregion

	private:

#pragma region private_classes

		struct ClientScene
		{
			pplx::task<Scene_ptr> task;
			bool connecting = false;
			bool isPublic = true;
		};

#pragma endregion

#pragma region private_methods

		Client(Configuration_ptr config);
		Client(const Client& other) = delete;
		Client& operator=(const Client& other) = delete;
		~Client();
		void initialize();
		pplx::task<void> connectToScene(const std::string& sceneId, const std::string& sceneToken, const std::vector<Route_ptr>& localRoutes);
		pplx::task<void> disconnect(Scene_ptr scene);
		pplx::task<void> disconnectAllScenes();
		pplx::task<void> destroy();
		pplx::task<Scene_ptr> getSceneInternal(const std::string& sceneId, const SceneEndpoint& sceneEndpoint);
		void transport_packetReceived(Packet_ptr packet);
		pplx::task<void> updateServerMetadata();
		pplx::task<void> ensureConnectedToServer(const SceneEndpoint& sceneEndpoint);
		void dispatchEvent(const std::function<void(void)>& ev);
		void setConnectionState(ConnectionState state);







		template<typename T1, typename T2>
		pplx::task<T2> sendSystemRequest(byte id, const T1& parameter)
		{
			auto peer = _serverConnection.lock();
			if (!peer)
			{
				return pplx::task_from_exception<T2>(std::runtime_error("Peer disconnected"));
			}
			auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();
			return requestProcessor->sendSystemRequest<T1, T2>(peer.get(), id, parameter);
		}

#pragma endregion

#pragma region private_members

		std::shared_ptr<DependencyResolver> _dependencyResolver;
		std::mutex _connectionStateMutex;
		ConnectionState _connectionState = ConnectionState::Disconnected;
		bool _connectionStateObservableCompleted = false;
		rxcpp::subjects::subject<ConnectionState> _connectionStateObservable;
		pplx::task<void> _currentTask = pplx::task_from_result();
		bool _initialized = false;
		std::string _accountId;
		std::string _applicationName;
		std::weak_ptr<IConnection>  _serverConnection;
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		std::map<std::string, std::string> _metadata;
		double _offset = 0;
		bool _synchronisedClock = true;
		std::unordered_map<std::string, ClientScene> _scenes;
		std::mutex _scenesMutex;
		std::vector<IPlugin*> _plugins;
		std::mutex _connectionMutex;
		pplx::task<void> _connectionTask;
		pplx::task_completion_event<void> _disconnectionTce;
		bool _connectionTaskSet = false;
		Action<ConnectionState> _onConnectionStateChanged;
		std::shared_ptr<Configuration> _config;
		rxcpp::composite_subscription _connectionSubscription;
		Serializer _serializer;

#pragma endregion
	};
};
