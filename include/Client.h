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
			void closeConnection(IConnection* connection, wstring reason);

		private:
			uint64 _current = 0;
		};

	public:

		/*! Constructor.
		\param config A pointer to a client configuration.
		*/
		STORMANCER_DLL_API Client(Configuration* config);

		/*! Destructor.
		*/
		STORMANCER_DLL_API ~Client();

		Client(Client& other) = delete;
		Client& operator=(Client& other) = delete;

	public:

		/*! Get a public scene.
		\param sceneId The scene Id as a string.
		\param userData Some custom user data as a string.
		\return a task to the connection to the scene.
		*/
		STORMANCER_DLL_API pplx::task<shared_ptr<Scene>> getPublicScene(wstring sceneId, wstring userData = L"");

		void initialize();
		wstring applicationName();
		ILogger* logger();
		void setLogger(ILogger* logger);
		pplx::task<shared_ptr<Scene>> getScene(wstring sceneId, SceneEndpoint sep);
		void disconnect();

	private:
		pplx::task<void> connectToScene(Scene* scene, wstring& token, vector<Route*> localRoutes);
		pplx::task<void> disconnect(Scene* scene, byte sceneHandle);
		void disconnectAllScenes();
		void transport_packetReceived(shared_ptr<Packet<>> packet);

		template<typename T1, typename T2>
		pplx::task<T2> sendSystemRequest(byte id, T1& parameter)
		{
			int a = 0;
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				parameter.serialize(stream); // serialize request dto
			}).then([this](shared_ptr<Packet<>> packet) {
				auto r = T2(packet->stream); // deserialize result dto
				return r;
			});
		}

		template<>
		pplx::task<EmptyDto> sendSystemRequest<byte, EmptyDto>(byte id, byte& parameter)
		{
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				ISerializable::serialize(byte(parameter), stream); // serialize byte
			}).then([this](shared_ptr<Packet<>> packet) {
				return EmptyDto(packet->stream); // deserialize result dto
			});
		}

	private:
		bool _initialized = false;
		ILogger* _logger = nullptr;
		wstring _accountId;
		wstring _applicationName;
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
