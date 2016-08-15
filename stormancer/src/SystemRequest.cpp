#include "stormancer.h"

namespace Stormancer
{
	SystemRequest::SystemRequest(pplx::task_completion_event<Packet_ptr> tce)
		: tce(tce)
	{
	}

	SystemRequest::~SystemRequest()
	{
	}
};
