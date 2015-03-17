#pragma once
#include "stdafx.h"
#include "Configuration/ClientConfiguration.h"

namespace Stormancer
{
	class Client
	{
	public:
		Client(ClientConfiguration config);
		~Client();

		void initialize();

	private:
		std::map<std::string, ISerializer&> _serializers;
		std::map<std::string, std::string> _metadata;
		std::string _accountId;
		std::string _applicationName;
		ApiClient _apiClient;
		ItokenHandler _tokenHandler;
		_transport;
		_dispatcher;
		_requestProcessor;
		_scenesDispatcher;
		_maxPeers;
	};
};
