#pragma once
#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	/// System request.
	class SystemRequest
	{
	public:
		SystemRequest(pplx::task_completion_event<Packet_ptr> tce);
		virtual ~SystemRequest();

	public:
		uint16 id = 0;
		time_t lastRefresh = time(NULL);
		pplx::task_completion_event<Packet_ptr> tce;
	};
};
