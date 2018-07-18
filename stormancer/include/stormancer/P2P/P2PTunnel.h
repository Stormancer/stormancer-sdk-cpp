#pragma once

#include "stormancer/headers.h"
#include "stormancer/P2P/P2PEnums.h"

namespace Stormancer
{
	class P2PTunnel
	{
	public:

#pragma region public_methods

		P2PTunnel(std::function<void(void)> onRelease);
		~P2PTunnel();

#pragma endregion

#pragma region public_members

		std::string ip;
		uint16 port;
		P2PTunnelSide side;
		std::string id;

#pragma endregion

	private:

#pragma region private_members

		std::function<void(void)> _onRelease;

#pragma endregion
	};
}
