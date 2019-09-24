#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/ITokenHandler.h"
#include "stormancer/Serializer.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	/// Manade a token.
	class TokenHandler : public ITokenHandler
	{
	public:

#pragma region public_methods

		TokenHandler(ILogger_ptr logger);
		virtual ~TokenHandler();
		SceneEndpoint decodeToken(const std::string& token) override;
		SceneEndpoint getSceneEndpointInfo(const std::string& token) override;

#pragma endregion

#pragma region private_members

		Serializer _serializer;
		ILogger_ptr _logger;

#pragma endregion
	};
}
