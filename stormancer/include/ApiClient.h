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
		ApiClient(std::shared_ptr<Configuration> config, ITokenHandler* tokenHandler);
		~ApiClient();

		pplx::task<SceneEndpoint> getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId);
		
	private:
		pplx::task<SceneEndpoint> getSceneEndpointImpl(std::vector<std::string>  endpoints, std::shared_ptr<std::vector<std::string>> errors, std::string accountId, std::string applicationName, std::string sceneId);

		std::shared_ptr<Configuration> _config;
		ITokenHandler* _tokenHandler;
	};
};
