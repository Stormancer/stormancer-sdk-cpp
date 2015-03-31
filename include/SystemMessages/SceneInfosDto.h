#pragma once
#include "headers.h"
#include "Core/Models/RouteDto.h"

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
