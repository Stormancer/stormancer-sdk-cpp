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
		ApiClient(Configuration_ptr config, ITokenHandler* tokenHandler);
		~ApiClient();

		pplx::task<SceneEndpoint> getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId, std::string userData = std::string());

	private:
		Configuration_ptr _config;
		ITokenHandler* _tokenHandler;
	};
};
