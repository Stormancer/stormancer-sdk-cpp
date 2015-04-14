#pragma once
#include "headers.h"
#include "Configuration/ClientConfiguration.h"
#include "ApiClient.h"
#include "SceneEndpoint.h"
#include "Core/ISerializer.h"
#include "Scene.h"
#include "Core/ILogger.h"
#include "Processors/SceneDispatcher.h"
#include "Processors/RequestProcessor.h"

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
		Client(ClientConfiguration* config);
		~Client();

	public:
		void initialize();
		wstring applicationName();
		ILogger* logger();
		void setLogger(ILogger* logger);
		pplx::task<Scene*> getPublicScene(wstring sceneId, wstring userData);
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
				this->_systemSerializer->serialize(parameter, stream);
			}).then([this](Packet<>* packet) {
				U res;
				this->_systemSerializer->deserialize(packet->stream, res);
				return res;
			});
		}

	private:
		bool _initialized = false;
		wstring _accountId;
		wstring _applicationName;
		//const PluginBuildContext _pluginCtx;
		IConnection* _serverConnection;
		ITransport* _transport;
		IPacketDispatcher* _dispatcher;
		ITokenHandler* _tokenHandler;
		ApiClient* _apiClient;
		ISerializer* _systemSerializer;
		RequestProcessor* _requestProcessor;
		SceneDispatcher* _scenesDispatcher;
		map<wstring, ISerializer*> _serializers;
		pplx::cancellation_token_source _cts;
		uint16 _maxPeers;
		stringMap _metadata;
		ILogger* _logger;
	};
};
