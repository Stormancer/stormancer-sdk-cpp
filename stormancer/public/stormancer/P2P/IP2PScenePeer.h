#pragma once
#include "stormancer/IScenePeer.h"
#include "stormancer/P2P/P2PTunnel.h"
#include "pplx/pplxtasks.h"

namespace Stormancer
{
	class IP2PScenePeer :public IScenePeer
	{
	public:
		virtual pplx::task<std::shared_ptr<P2PTunnel>> openP2PTunnel(const std::string& serverId) = 0;
	};
};