#pragma once
#include "libs.h"
#include "Configuration/ClientConfiguration.h"
#include "Infrastructure/ITokenHandler.h"

namespace Stormancer
{
	class ApiClient
	{
	public:
		ApiClient(/*ClientConfiguration config, shared_ptr<ITokenHandler*> tokenHandler*/);
		~ApiClient();

#pragma region methods
		template<typename T>
		pplx::task<SceneEndpoint> getSceneEndpoint(string accountId, string applicationName, string sceneId, T userData = string());
#pragma endregion

#pragma region attributes
	private:
		//ClientConfiguration _config;
		const std::string _createTokenUri;
		const shared_ptr<ITokenHandler*> _tokenHandler;
#pragma endregion
	};
};
