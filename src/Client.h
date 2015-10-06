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

namespace Stormancer
{
	/// Manage the connection to the scenes of an application.
	class Client
	{
		/// The scene need to access some private method of the client.
		friend class Scene;

	private:
		class Watch
		{
		public:
			Watch();
			~Watch();

		public:
			void reset();
			int64 getElapsedTime();
			void setBaseTime(int64 baseTime);

		private:
			std::chrono::steady_clock::time_point _startTime;
			int64 _baseTime = 0;
		};

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

		private:
			uint64 _current = 0;
		};

	public:

		/// Constructor.
		/// \param config A pointer to a Configuration.
		STORMANCER_DLL_API Client(Configuration* config);

		/// Destructor.
		STORMANCER_DLL_API ~Client();

		/// Copy constructor deleted.
		Client(Client& other) = delete;

		/// Copy operator deleted.
		Client& operator=(Client& other) = delete;

	public:

		/// Get a public scene.
		/// \param sceneId The scene Id as a string.
		/// \param userData Some custom user data as a string.
		/// \return a task to the connection to the scene.
		STORMANCER_DLL_API pplx::task<Scene_ptr> getPublicScene(std::string sceneId, std::string userData = "");

		STORMANCER_DLL_API pplx::task<Scene_ptr> getScene(std::string token);

		/// Returns the name of the application.
		STORMANCER_DLL_API std::string applicationName();

		/// Returns the used logger.
		STORMANCER_DLL_API ILogger* logger();

		/// Set the logger
		STORMANCER_DLL_API void setLogger(ILogger* logger);

		/// Disconnect and close all connections.
		/// this method returns nothing. So it's useful for application close.
		STORMANCER_DLL_API void disconnect();

		STORMANCER_DLL_API int64 clock();

		STORMANCER_DLL_API int64 lastPing();

	private:

		void initialize();

		/// Connect to a scene
		/// \param scene The scene to connect to.
		/// \param token Application token.
		/// \param localRoutes Local routes declared.
		/// \return A task which complete when the connection is done.
		pplx::task<void> connectToScene(Scene* scene, std::string& token, std::vector<Route_ptr> localRoutes);

		/// Disconnect from a scene.
		/// \param scene The scene.
		/// \param sceneHandle Scene handle.
		/// \return A task which complete when the disconnection is done.
		pplx::task<void> disconnect(Scene* scene, byte sceneHandle);

		/// Get scene informations for connection.
		/// \param sceneId Scene id.
		/// \param sep Scene Endpoint retrieved by ApiClient::getSceneEndpoint
		/// \return A task which complete when the scene informations are retrieved.
		pplx::task<Scene_ptr> getScene(std::string sceneId, SceneEndpoint sep);

		/// Disconnect from all scenes.
		void disconnectAllScenes();

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
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				// serialize request dto
				msgpack::pack(stream, parameter);
				//auto res = stream->str();
			}).then([this](Packet_ptr packet) {
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

		void startSyncClock();

		void stopSyncClock();

		pplx::task<void> syncClockImpl();

	private:
		bool _initialized = false;
		ILogger* _logger = nullptr;
		std::string _accountId;
		std::string _applicationName;
		IConnection* _serverConnection = nullptr;
		IScheduler* _scheduler = nullptr;
		ITransport* _transport = nullptr;
		ITokenHandler* _tokenHandler = nullptr;
		ApiClient* _apiClient = nullptr;
		RequestProcessor* _requestProcessor = nullptr;
		SceneDispatcher* _scenesDispatcher = nullptr;
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		stringMap _metadata;
		IPacketDispatcher* _dispatcher = nullptr;
		Watch _watch;
		int64 _offset = 0;
		int64 _lastPing = 0;
		int64 _pingInterval = 5000;
		Subscription_ptr _syncClockSubscription;
		PluginBuildContext _pluginCtx;
		std::map<std::string, std::weak_ptr<Scene>> _scenes;
	};
};
