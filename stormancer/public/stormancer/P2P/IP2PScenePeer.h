#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/IScenePeer.h"
#include "stormancer/P2P/P2PTunnel.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	class IP2PScenePeer :public IScenePeer
	{
	public:
		virtual pplx::task<std::shared_ptr<P2PTunnel>> openP2PTunnel(const std::string& serverId, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
	};
};