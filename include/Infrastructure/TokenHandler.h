#pragma once
#include "headers.h"
#include "Infrastructure/ITokenHandler.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	class TokenHandler : public ITokenHandler
	{
	public:
		TokenHandler();
		virtual ~TokenHandler();

		SceneEndpoint* decodeToken(wstring token);

	private:
		const ISerializer* _tokenSerializer;
	};
};
