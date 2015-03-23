#pragma once
#include "libs.h"
#include "Core/Packet.h"

namespace Stormancer
{
	using handlerFunction = function < bool(Packet2) > ;
	using processorFunction = function < bool(byte, Packet2) > ;
	using handlerMap = map < byte, handlerFunction > ;
}
