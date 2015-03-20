#pragma once
#include "libs.h"
#include "Configuration/ClientConfiguration.h"
#include "Infrastructure/ITokenHandler.h"

namespace Stormancer
{
	class ApiClient
	{
	public:
		ApiClient(ClientConfiguration config, ITokenHandler tokenHandler);
		~ApiClient();

	private:
		ClientConfiguration _config;
		const std::string _createTokenUri;
		const ITokenHandler _tokenHandler;
	};
};
