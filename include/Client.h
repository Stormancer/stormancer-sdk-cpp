#pragma once
#include "headers.h"
#include "Configuration/ClientConfiguration.h"
#include "ApiClient.h"
#include "Scene.h"
#include "SceneEndpoint.h"

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

		void initialize();
		wstring applicationName();
		//shared_ptr<ILogger*> logger();
		//void setLogger(shared_ptr<ILogger*> logger);

		task<Scene> getPublicScene(wstring sceneId, wstring userData);

	private:
		task<Scene> getScene(wstring sceneId, SceneEndpoint sep);

	private:
		bool _initialized;
		wstring _accountId;
		wstring _applicationName;
		//const PluginBuildContext _pluginCtx;
		shared_ptr<IConnection> _serverConnection;
		shared_ptr<ITransport> _transport;
		//shared_ptr<IPacketDispatcher> _dispatcher;
		shared_ptr<ITokenHandler> _tokenHandler;
		ApiClient _apiClient;
		//shared_ptr<ISerializer*> _systemSerializer;
		//RequestProcessor _requestProcessor;
		//SceneDispatcher _sceneDispatcher;
		map<wstring, shared_ptr<ISerializer>> _serializers;
		cancellation_token_source _cts;
		uint16 _maxPeers;
		StringMap _metadata;
		//shared_ptr<ILogger> _logger;
	};
};
