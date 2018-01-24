#pragma once

#include "headers.h"
#include "ITokenHandler.h"
#include "Serializer.h"

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
