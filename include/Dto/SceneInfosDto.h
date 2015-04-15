#pragma once
#include "headers.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	struct SceneInfosDto
	{
		wstring SceneId;
		stringMap Metadata;
		vector<RouteDto> Routes;
		wstring SelectedSerializer;
	};
};
