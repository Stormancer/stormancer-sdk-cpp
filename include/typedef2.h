#pragma once
#include "Core/Packet.h"

namespace Stormancer
{
	using namespace std;

	using handlerFunction = function < bool(Stormancer::Packet2) >;
	using processorFunction = function < bool(byte, Stormancer::Packet2) >;
	using handlerMap = map < byte, handlerFunction >;
};
