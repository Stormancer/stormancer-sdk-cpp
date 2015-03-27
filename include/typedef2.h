#pragma once
#include "Core/Packet.h"

namespace Stormancer
{
	using namespace std;

	using handlerFunction = function < bool(Stormancer::Packet<>) >;
	using processorFunction = function < bool(byte, Stormancer::Packet<>) >;
	using handlerMap = map < byte, handlerFunction >;
};
