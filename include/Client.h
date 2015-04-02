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
	private:
		class ConnectionHandler : public IConnectionManager
		{
		public:
			ConnectionHandler();
			~ConnectionHandler();

		public:
			uint64 generateNewConnectionId();
			void newConnection(shared_ptr<IConnection> connection);
			shared_ptr<IConnection> getConnection(uint64 id);
			void closeConnection(shared_ptr<IConnection>, wstring reason);

		private:
			uint64 _current = 0;
		};

	public:
		Client(ClientConfiguration& config);
		~Client();

	public:
		void initialize();
		wstring applicationName();
		shared_ptr<ILogger> logger();
		void setLogger(shared_ptr<ILogger> logger);
		task<shared_ptr<Scene>> getPublicScene(wstring sceneId, wstring userData);
		task<shared_ptr<Scene>> getScene(wstring sceneId, SceneEndpoint sep);
		void disconnect();

	private:
		task<void> connectToScene(Scene* scene, wstring& token, vector<Route&> localRoutes);
		task<void> disconnect(Scene* scene, byte sceneHandle);

	private:
		template<typename T, typename U>
		task<U> sendSystemRequest(byte id, T parameter);

	private:
		bool _initialized;
		wstring _accountId;
		wstring _applicationName;
		//const PluginBuildContext _pluginCtx;
		shared_ptr<IConnection> _serverConnection;
		shared_ptr<ITransport> _transport;
		shared_ptr<IPacketDispatcher> _dispatcher;
		shared_ptr<ITokenHandler> _tokenHandler;
		ApiClient _apiClient;
		shared_ptr<ISerializer> _systemSerializer;
		RequestProcessor _requestProcessor;
		SceneDispatcher _scenesDispatcher;
		map<wstring, shared_ptr<ISerializer>> _serializers;
		cancellation_token_source _cts;
		uint16 _maxPeers;
		stringMap _metadata;
		shared_ptr<ILogger> _logger;
	};
};
