#pragma once
#include "headers.h"
#include "ITokenHandler.h"

namespace Stormancer
{
	class TokenHandler : public ITokenHandler
	{
	public:
		TokenHandler();
		virtual ~TokenHandler();

		SceneEndpoint* decodeToken(wstring& token);
	};
};
