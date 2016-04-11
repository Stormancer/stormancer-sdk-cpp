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

		pplx::task<SceneEndpoint> getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId, std::string userData = std::string());
		
	private:
		pplx::task<SceneEndpoint> getSceneEndpointImpl(std::vector<std::string> & endpoints, std::vector<std::string> &errors, std::string accountId, std::string applicationName, std::string sceneId, std::string userData = std::string());

		Configuration* _config;
		ITokenHandler* _tokenHandler;
	};
};
