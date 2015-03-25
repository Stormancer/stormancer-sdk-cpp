#pragma once
#include "headers.h"
#include "Configuration/ClientConfiguration.h"
#include "SceneEndpoint.h"

namespace Stormancer
{
	class ApiClient
	{
	public:
		ApiClient(ClientConfiguration& config/*, shared_ptr<ITokenHandler*> tokenHandler*/);
		~ApiClient();

		pplx::task<SceneEndpoint> getSceneEndpoint(string accountId, string applicationName, string sceneId, string userData = string());

	private:
		ClientConfiguration _config;
		string _createTokenUri;
		//const shared_ptr<ITokenHandler*> _tokenHandler;
	};
};
