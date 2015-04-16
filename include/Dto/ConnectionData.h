#pragma once
#include "headers.h"

namespace Stormancer
{
	struct ConnectionData
	{
		wstring AccountId;
		wstring Application;
		wstring ContentType;
		wstring DeploymentId;
		stringMap Endpoints;
		time_t Expiration;
		time_t Issued;
		wstring Routing;
		wstring SceneId;
		wstring UserData;
	};
};
