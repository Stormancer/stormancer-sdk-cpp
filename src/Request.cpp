#include "stormancer.h"

namespace Stormancer
{
	Request::Request(PacketObserver&& observer)
		: observer(observer)/*,
		task(std::move(create_task(tcs)))*/
	{
	}

	Request::~Request()
	{
	}
};
