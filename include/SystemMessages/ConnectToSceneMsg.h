#pragma once
#include "headers.h"
#include "Core/Models/RouteDto.h"

namespace Stormancer
{
	struct ConnectToSceneMsg
	{
		wstring Token;
		vector<RouteDto> Routes;
		stringMap ConnectionMetadata;
	};
};
