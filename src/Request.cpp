#include "stormancer.h"

namespace Stormancer
{
	Request::Request(PacketObserver&& observer)
		: observer(observer)
	{
	}

	Request::~Request()
	{
	}
};
