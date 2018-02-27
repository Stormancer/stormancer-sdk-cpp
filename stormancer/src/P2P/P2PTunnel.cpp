#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PTunnel.h"

namespace Stormancer
{
	P2PTunnel::P2PTunnel(std::function<void(void)> onRelease)
	{
		_onRelease = onRelease;
	}

	P2PTunnel::~P2PTunnel()
	{
		_onRelease();
	}
};
