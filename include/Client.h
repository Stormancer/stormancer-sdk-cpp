#pragma once
#include "stdafx.h"
#include "Configuration/ClientConfiguration.h"
#include "Core/ISerializer.h"
#include "ApiClient.h"
#include "Infrastructure/ITokenHandler.h"

namespace Stormancer
{
	class Client
	{
	private:
		class ConnectionHandler : public IConnectionManager
		{
		private:
			uint64 _current = 0;
			uint64 generateNewConnectionId()
			{
				// TODO
				return _current++;
			}
			void newConnection(IConnection& connection)
			{

			}
			IConnection& getConnection(uint64 id)
			{
				throw new exception("Not Implemented");
			}
			void closeConnection(IConnection& connection, string reason)
			{

			}
		};

	public:
		Client(ClientConfiguration& config);
		~Client();

		void initialize();

	private:
		map<string, ISerializer&> _serializers;
		map<string, string> _metadata;
		string _accountId;
		string _applicationName;
		ApiClient _apiClient;
		ITokenHandler _tokenHandler;
		_transport;
		_dispatcher;
		_requestProcessor;
		_scenesDispatcher;
		_maxPeers;
	};
};
