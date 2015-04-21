#pragma once
#include "headers.h"
#include "ClientConfiguration.h"
#include "ApiClient.h"
#include "SceneEndpoint.h"
#include "Scene.h"
#include "ILogger.h"
#include "SceneDispatcher.h"
#include "RequestProcessor.h"
#include "IConnection.h"

namespace Stormancer
{
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
		STORMANCER_DLL_API Client(ClientConfiguration* config);
		STORMANCER_DLL_API ~Client();

	public:
		void initialize();
		wstring applicationName();
		ILogger* logger();
		void setLogger(ILogger* logger);
		STORMANCER_DLL_API pplx::task<Scene*> getPublicScene(wstring sceneId, wstring userData);
		pplx::task<Scene*> getScene(wstring sceneId, SceneEndpoint* sep);
		void disconnect();

	private:
		pplx::task<void> connectToScene(Scene* scene, wstring& token, vector<Route*> localRoutes);
		pplx::task<void> disconnect(Scene* scene, byte sceneHandle);
		void transport_packetReceived(Packet<>* packet);

		template<typename T, typename U>
		pplx::task<U> sendSystemRequest(byte id, T& parameter)
		{
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, &parameter](bytestream* stream) {
				// serialize
				ISerializable::serialize(parameter, stream);
			}).then([this](Packet<>* packet) {
				// deserialize
				U res;
				ISerializable::deserialize(packet->stream, res);
				return res;
			});
		}

		template<typename U>
		pplx::task<U> sendSystemRequest(byte id, ISerializable* parameter)
		{
			return _requestProcessor->sendSystemRequest(_serverConnection, id, [this, parameter](bytestream* stream) {
				// serialize
				ISerializable::serialize(parameter, stream);
			}).then([this](Packet<>* packet) {
				// deserialize
				U res;
				ISerializable::deserialize(packet->stream, res);
				return res;
			});
		}

	private:
		bool _initialized = false;
		wstring _accountId;
		wstring _applicationName;
		//const PluginBuildContext _pluginCtx;
		IConnection* _serverConnection = nullptr;
		ITransport* _transport = nullptr;
		IPacketDispatcher* _dispatcher = nullptr;
		ITokenHandler* _tokenHandler = nullptr;
		ApiClient* _apiClient = nullptr;
		RequestProcessor* _requestProcessor = nullptr;
		SceneDispatcher* _scenesDispatcher = nullptr;
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers = 0;
		stringMap _metadata;
		ILogger* _logger = nullptr;
	};
};
