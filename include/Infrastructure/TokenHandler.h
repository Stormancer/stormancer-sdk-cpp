#pragma once
#include "libs.h"
#include "Infrastructure/ITokenHandler.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	class TokenHandler : public ITokenHandler
	{
	public:
		TokenHandler();
		virtual ~TokenHandler();

		SceneEndpoint& decodeToken(string token);

	private:
		const shared_ptr<ISerializer*> _tokenSerializer;
	};
};
