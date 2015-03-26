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
		StringMap Endpoints;
		wstring AccountId;
		wstring Application;
		wstring SceneId;
		wstring Routing;
		time_t Issued;
		time_t Expiration;
		byte* UserData;
		wstring ContentType;
	};
};
