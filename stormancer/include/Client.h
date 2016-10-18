#pragma once
#include "headers.h"
#include "Configuration.h"
#include "ApiClient.h"
#include "SceneEndpoint.h"
#include "Scene.h"
#include "ILogger.h"
#include "SceneDispatcher.h"
#include "RequestProcessor.h"
#include "IConnection.h"
#include "IScheduler.h"
#include "Watch.h"
#include "ITokenHandler.h"

namespace Stormancer
{
	class Client;

	/// Manage the connection to the scenes of an application.
	class Client
	{
		/// The scene need to access some private method of the client.
		friend class Scene;

	private:
		class ConnectionHandler : public IConnectionManager
		{
		public:
			ConnectionHandler();
			~ConnectionHandler();

		public:
			uint64 generateNewConnectionId();
			void newConnection(IConnection* connection);
			IConnection* getConnection(uint64 id);
			void closeConnection(IConnection* connection, std::string reason);
		};

		

	private:
		Client(std::shared_ptr<Configuration> config);

	public:
		/// Destructor.
		

	private:
		/// Copy constructor deleted.
		Client(Client& other) = delete;

		/// Copy operator deleted.
		Client& operator=(Client& other) = delete;
		~Client();
	public:
		/// Factory.
		/// \param config A pointer to a Configuration.
		static Client* createClient(std::shared_ptr<Configuration> config);
		static pplx::task<void> destroy(Client* client);
		/// Get a public scene.
		/// \param sceneId The scene Id as a string.
		/// \param userData Some custom user data as a string.
		/// \return a task to the connection to the scene.
		pplx::task<std::shared_ptr<Scene>> getPublicScene(const char* sceneId);

		pplx::task<std::shared_ptr<Scene>> getScene(const char* token);

		/// Returns the name of the application.
		std::string applicationName();

		/// Returns the used logger.
		std::shared_ptr<ILogger> logger();



		/// Disconnect and close all connections.
		/// this method returns nothing. So it's useful for application close.
		pplx::task<void> disconnect(bool immediate = false);

		int64 clock();

		int64 lastPing();

		DependencyResolver* dependencyResolver() const;

		rxcpp::observable<ConnectionState> GetConnectionStateChangedObservable();

		ConnectionState GetCurrentConnectionState() const;

		void SetMedatata(const std::string key, const std::string value);
	private:

		void initialize();

		/// Connect to a scene
		/// \param scene The scene to connect to.
		/// \param token Application token.
		/// \param localRoutes Local routes declared.
		/// \return A task which complete when the connection is done.
		pplx::task<void> connectToScene(std::shared_ptr<Scene> scene, std::string& token, std::vector<Route_ptr> localRoutes);

		/// Disconnect from a scene.
		/// \param scene The scene.
		/// \param sceneHandle Scene handle.
		/// \return A task which complete when the disconnection is done.
		pplx::task<void> disconnect(Scene* scene, byte sceneHandle, bool immediate = false);

		/// Disconnect from all scenes.
		pplx::task<void> disconnectAllScenes(bool immediate = false);

		/// Get scene informations for connection.
		/// \param sceneId Scene id.
		/// \param sep Scene Endpoint retrieved by ApiClient::getSceneEndpoint
		/// \return A task which complete when the scene informations are retrieved.
		pplx::task<std::shared_ptr<Scene>> getScene(std::string sceneId, SceneEndpoint sep);

		/// Called by the transport when a packet has been received.
		/// \param packet Received packet.
		void transport_packetReceived(Packet_ptr packet);

		/// Send a system request to the server for important operations.
		/// \param id Request id.
		/// \param parameter Data to send with the request.
		/// \return A task which complete when the response is available (As result of the task).
		template<typename T1, typename T2>
		pplx::task<T2> sendSystemRequest(byte id, T1& parameter)
		{
			return dependencyResolver()->resolve<RequestProcessor>()->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				// serialize request dto
				msgpack::pack(stream, parameter);
				//auto res = stream->str();
			}).then([this](pplx::task<Packet_ptr> t) {
				Packet_ptr packet;

				try
				{
					packet = t.get();
				}
				catch (const std::exception& ex)
				{
					throw std::runtime_error(std::string() + "System request failed (" + ex.what() + ")");
				}

				// deserialize result dto
				std::string buffer;
				*packet->stream >> buffer;
				msgpack::unpacked result;
				msgpack::unpack(&result, buffer.data(), buffer.size());
				T2 t2;
				result.get().convert(&t2);
				return t2;
			});
		}

		pplx::task<void> updateServerMetadata();
		pplx::task<void> tryConnectToServer(SceneEndpoint endpoint);
		

		void dispatchEvent(std::function<void(void)> ev);

		void setConnectionState(ConnectionState state);
	private:
		//Connection state
		ConnectionState _connectionState = ConnectionState::Disconnected;
		rxcpp::subjects::subject<ConnectionState> _connectionStateObservable;
		
		//completed task
		pplx::task<void> _currentTask = pplx::task_from_result();

		bool _initialized = false;
		std::shared_ptr<ILogger> _logger = nullptr;
		std::string _accountId;
		std::string _applicationName;
		IConnection* _serverConnection = nullptr;
		
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		stringMap _metadata;
		
		double _offset = 0;
		
		bool _synchronisedClock = true;
		
		
		
		
		
		std::map<std::string, ScenePtr> _scenes;
		DependencyResolver* _dependencyResolver = nullptr;
		std::vector<IPlugin*> _plugins;
		std::mutex _connectionMutex;
		pplx::task<void> _connectionTask;
		bool _connectionTaskSet = false;
		Action<ConnectionState> _onConnectionStateChanged;
		std::shared_ptr<Configuration> _config = nullptr;
	};
};
