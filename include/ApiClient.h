#pragma once
#include "headers.h"
#include "Configuration/ClientConfiguration.h"

namespace Stormancer
{
	class ApiClient
	{
	public:
		ApiClient(ClientConfiguration& config/*, shared_ptr<ITokenHandler*> tokenHandler*/);
		~ApiClient();

		//template<typename T>
		//pplx::task<SceneEndpoint> getSceneEndpoint(string accountId, string applicationName, string sceneId, T userData = string());

	private:
		ClientConfiguration _config;
		const string _createTokenUri;
		//const shared_ptr<ITokenHandler*> _tokenHandler;
	};
};
