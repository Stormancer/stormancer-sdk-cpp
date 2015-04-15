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
		wstring Expiration;
		wstring Issued;
		wstring Routing;
		wstring SceneId;
		vector<byte> UserData;
	};
};
