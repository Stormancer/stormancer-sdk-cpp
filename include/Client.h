#pragma once
#include "libs.h"
#include "Configuration/ClientConfiguration.h"
#include "Core/ISerializer.h"
#include "ApiClient.h"
#include "Infrastructure/ITokenHandler.h"
#include "IConnectionManager.h"

namespace Stormancer
{
	class Client
	{
	private:
		class ConnectionHandler : public IConnectionManager
		{
		public:
			uint64 generateNewConnectionId();
			void newConnection(shared_ptr<IConnection*> connection);
			shared_ptr<IConnection*> getConnection(uint64 id);
			void closeConnection(shared_ptr<IConnection*>, string reason);

		private:
			uint64 _current = 0;
		};

	public:
		Client(ClientConfiguration& config);
		~Client();

		void initialize();
		string applicationName();

	private:
		const ApiClient _apiClient;
		const string _accountId;
		const string _applicationName;
		const PluginBuildContext _pluginCtx;
		shared_ptr<IConnection*> _serverConnection;
		shared_ptr<ITransport*> _transport;
		shared_ptr<IPacketDispatcher*> _dispatcher;
		bool _initialized;
		shared_ptr<ITokenHandler*> _tokenHandler;
		map<string, TaskCompletionSource<bool>> _pendingTasksTcs;
		map<string, ISerializer&> _serializers;
		map<string, string> _metadata;
		_requestProcessor;
		_scenesDispatcher;
		_maxPeers;
	};
};
