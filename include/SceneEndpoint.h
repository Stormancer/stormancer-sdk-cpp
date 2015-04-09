#pragma once
#include "headers.h"
#include "Core/Models/ConnectionData.h"

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
