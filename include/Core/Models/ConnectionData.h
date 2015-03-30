#pragma once
#include "headers.h"

namespace Stormancer
{
	struct ConnectionData
	{
	public:
		ConnectionData();
		~ConnectionData();

	public:
		wstring AccountId;
		wstring Application;
		wstring ContentType;
		wstring DeploymentId;
		StringMap Endpoints;
		wstring Expiration;
		wstring Issued;
		wstring Routing;
		wstring SceneId;
		vector<byte> UserData;
	};
};
