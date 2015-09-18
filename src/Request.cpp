#include "stormancer.h"

namespace Stormancer
{
	Request::Request(pplx::task_completion_event<Packet_ptr> tce)
		: tce(tce)
	{
	}

	Request::~Request()
	{
	}
};
