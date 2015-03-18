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
	public:
		Client(ClientConfiguration config);
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
