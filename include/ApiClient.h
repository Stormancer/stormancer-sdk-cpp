#pragma once
#include "headers.h"
#include "ClientConfiguration.h"
#include "SceneEndpoint.h"
#include "ITokenHandler.h"

namespace Stormancer
{
	class ApiClient
	{
	public:
		ApiClient(ClientConfiguration* config, ITokenHandler* tokenHandler);
		~ApiClient();

		pplx::task<SceneEndpoint*> getSceneEndpoint(wstring accountId, wstring applicationName, wstring sceneId, wstring userData = wstring());

	private:
		ClientConfiguration* _config;
		wstring _createTokenUri;
		ITokenHandler* _tokenHandler;
	};
};
