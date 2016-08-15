#pragma once
#include "headers.h"
#include "ITokenHandler.h"

namespace Stormancer
{
	/// Manade a token.
	class TokenHandler : public ITokenHandler
	{
	public:
		TokenHandler();
		virtual ~TokenHandler();

		SceneEndpoint decodeToken(std::string& token);
	};
};
