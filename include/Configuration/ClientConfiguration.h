#pragma once
#include "stdafx.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		~ClientConfiguration();

		static ClientConfiguration forAccount(string account, string application);

	private:
		ClientConfiguration();
		ClientConfiguration(string account, string application, bool isLocalDev = false);

	public:
		string account;
		string application;
		string serverEndpoint;
		bool isLocalDev;
	};
};
