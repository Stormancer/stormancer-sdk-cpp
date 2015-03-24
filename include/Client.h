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
		/*class ConnectionHandler : public IConnectionManager
		{
		public:
			uint64 generateNewConnectionId();
			void newConnection(shared_ptr<IConnection*> connection);
			shared_ptr<IConnection*> getConnection(uint64 id);
			void closeConnection(shared_ptr<IConnection*>, string reason);

		private:
			uint64 _current = 0;
		};*/

	public:
		Client(ClientConfiguration& config);
		~Client();

		void initialize();
		string applicationName();
		//shared_ptr<ILogger*> logger();
		//void setLogger(shared_ptr<ILogger*> logger);

		template<typename T>
		pplx::task<Scene> getPublicScene(string sceneId, T userData);

	private:
		pplx::task<Scene> getScene(string sceneId, SceneEndpoint sep);

	private:
		const ApiClient _apiClient;
		const string _accountId;
		const string _applicationName;
		//const PluginBuildContext _pluginCtx;
		//shared_ptr<IConnection*> _serverConnection;
		//shared_ptr<ITransport*> _transport;
		//shared_ptr<IPacketDispatcher*> _dispatcher;
		bool _initialized;
		//shared_ptr<ITokenHandler*> _tokenHandler;
		//shared_ptr<ISerializer*> _systemSerializer;
		//RequestProcessor _requestProcessor;
		//SceneDispatcher _sceneDispatcher;
		//map<string, shared_ptr<ISerializer*>> _serializers;
		//pplx::cancellation_token_source _cts;
		uint16 _maxPeers;
		StringMap _metadata;
		//shared_ptr<ILogger*> _logger;
	};
};
