#include "ApiClient.h"

namespace Stormancer
{
	ApiClient::ApiClient(ClientConfiguration config, ITokenHandler tokenHandler)
		: _config(config),
		_createTokenUri("%1/%2/scenes/%3/token"),
		_tokenHandler(tokenHandler)
	{
	}

	ApiClient::~ApiClient()
	{
	}
};
