#pragma once
#include "headers.h"
#include "Configuration/ClientConfiguration.h"
#include "SceneEndpoint.h"
#include "Infrastructure/ITokenHandler.h"

namespace Stormancer
{
	using namespace pplx;

	class ApiClient
	{
	public:
		ApiClient(ClientConfiguration& config, shared_ptr<ITokenHandler> tokenHandler);
		~ApiClient();

		task<SceneEndpoint> getSceneEndpoint(wstring accountId, wstring applicationName, wstring sceneId, wstring userData = wstring());

	private:
		ClientConfiguration _config;
		wstring _createTokenUri;
		const shared_ptr<ITokenHandler> _tokenHandler;
	};
};
