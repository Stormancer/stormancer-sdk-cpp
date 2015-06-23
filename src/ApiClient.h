#pragma once
#include "headers.h"
#include "Configuration.h"
#include "SceneEndpoint.h"
#include "ITokenHandler.h"

namespace Stormancer
{
	/// Retrieve informations from the Api.
	class ApiClient
	{
	public:
		ApiClient(Configuration* config, ITokenHandler* tokenHandler);
		~ApiClient();

		pplx::task<SceneEndpoint> getSceneEndpoint(wstring accountId, wstring applicationName, wstring sceneId, wstring userData = wstring());

	private:
		Configuration* _config;
		wstring _createTokenUri;
		ITokenHandler* _tokenHandler;
	};
};
