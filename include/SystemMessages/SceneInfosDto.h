#pragma once
#include "headers.h"
#include "Core/Models/RouteDto.h"

namespace Stormancer
{
	struct SceneInfosDto
	{
		wstring SceneId;
		StringMap Metadata;
		vector<RouteDto> Routes;
		wstring SelectedSerializer;
	};
};
