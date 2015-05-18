#pragma once
#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	using PacketObserver = rx::observer < shared_ptr<Packet<>> > ;

	class Request
	{
	public:
		Request(PacketObserver&& observer);
		virtual ~Request();

	public:
		uint16 id = 0;
		time_t lastRefresh = time(NULL);
		PacketObserver observer;
		//pplx::task_completion_event<void> tcs;
		//pplx::task<void> task;
	};
};
