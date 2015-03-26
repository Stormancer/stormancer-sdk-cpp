#pragma once
#include "headers.h"
#include "SceneEndpoint.h"

namespace Stormancer
{
	class ITokenHandler
	{
	public:
		ITokenHandler();
		virtual ~ITokenHandler();

		virtual SceneEndpoint decodeToken(wstring token) = 0;
	};
};
