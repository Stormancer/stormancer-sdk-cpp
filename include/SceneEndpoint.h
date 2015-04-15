#pragma once
#include "headers.h"
#include "Dto/ConnectionData.h"

namespace Stormancer
{
	class SceneEndpoint
	{
	public:
		SceneEndpoint();
		virtual ~SceneEndpoint();

	public:
		ConnectionData* tokenData;
		wstring token;
	};
};
