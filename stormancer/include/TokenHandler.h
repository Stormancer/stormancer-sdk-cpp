#pragma once

#include "headers.h"
#include "ITokenHandler.h"

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
	};
};
