#pragma once
#include "headers.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	struct ConnectToSceneMsg
	{
		wstring Token;
		vector<RouteDto> Routes;
		stringMap ConnectionMetadata;
	};
};
