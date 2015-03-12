#include "stdafx.h"
#include "ApiClient.h"


ApiClient::ApiClient(Configuration config, ITokenHandler tokenHandler)
	: _config(config),
	_createTokenUri("{0}/{1}/scenes/{2}/token"),
	_tokenHandler(tokenHandler)
{
}


ApiClient::~ApiClient()
{
}
