#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	pplx::task<std::string> GetXboxLiveAuthToken(std::string relyingPartyUrl, std::string account, std::string application, std::string token, std::string signature);
}