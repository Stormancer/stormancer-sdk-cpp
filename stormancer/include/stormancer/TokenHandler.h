#pragma once

#include "stormancer/headers.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	/// Manade a token.
	class TokenHandler : public ITokenHandler
	{
	public:

#pragma region public_methods

		TokenHandler();
		virtual ~TokenHandler();
		SceneEndpoint decodeToken(const std::string& token);

#pragma endregion

#pragma region private_members

		Serializer _serializer;

#pragma endregion
	};
};
