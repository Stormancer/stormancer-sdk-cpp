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

namespace Stormancer
{
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
		STORMANCER_DLL_API pplx::task<std::shared_ptr<Scene>> getPublicScene(std::string sceneId, std::string userData = "");

		/// Initialize packets reception from the right transport.
		void initialize();

		/// Returns the name of the application.
		std::string applicationName();

		/// Returns the used logger.
		ILogger* logger();

		/// Set the logger
		void setLogger(ILogger* logger);

		/// Get scene informations for connection.
		/// \param sceneId Scene id.
		/// \param sep Scene Endpoint retrieved by ApiClient::getSceneEndpoint
		/// \return A task which complete when the scene informations are retrieved.
		pplx::task<std::shared_ptr<Scene>> getScene(std::string sceneId, SceneEndpoint sep);

		/// Disconnect and close all connections.
		/// this method returns nothing. So it's useful for application close.
		void disconnect();

	private:

		/// Connect to a scene
		/// \param scene The scene to connect to.
		/// \param token Application token.
		/// \param localRoutes Local routes declared.
		/// \return A task which complete when the connection is done.
		pplx::task<void> connectToScene(Scene* scene, std::string& token, std::vector<Route*> localRoutes);

		/// Disconnect from a scene.
		/// \param scene The scene.
		/// \param sceneHandle Scene handle.
		/// \return A task which complete when the disconnection is done.
		pplx::task<void> disconnect(Scene* scene, byte sceneHandle);

		/// Disconnect from all scenes.
		void disconnectAllScenes();

		/// Called by the transport when a packet has been received.
		/// \param packet Received packet.
		void transport_packetReceived(std::shared_ptr<Packet<>> packet);

		/// Send a system request to the server for important operations.
		/// \param id Request id.
		/// \param parameter Data to send with the request.
		/// \return A task which complete when the response is available (As result of the task).
		template<typename T1, typename T2>
		pplx::task<T2> sendSystemRequest(byte id, T1& parameter)
		{
			_logger->log(LogLevel::Trace, "Client::sendSystemRequest", "", stringFormat(id));
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				_logger->log(LogLevel::Trace, "Client::sendSystemRequest", "lambda called by _requestProcessor::sendSystemRequest", "");
				// serialize request dto
				msgpack::pack(stream, parameter);
				auto res = stream->str();
			}).then([this](std::shared_ptr<Packet<>> packet) {
				_logger->log(LogLevel::Trace, "Client::sendSystemRequest", "systemRequest response", "");
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

	private:
		bool _initialized = false;
		ILogger* _logger = nullptr;
		std::string _accountId;
		std::string _applicationName;
		//const PluginBuildContext _pluginCtx;
		IConnection* _serverConnection = nullptr;
		ITransport* _transport = nullptr;
		ITokenHandler* _tokenHandler = nullptr;
		ApiClient* _apiClient = nullptr;
		RequestProcessor* _requestProcessor = nullptr;
		SceneDispatcher* _scenesDispatcher = nullptr;
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		stringMap _metadata;
		IPacketDispatcher* _dispatcher = nullptr;
	};
};
