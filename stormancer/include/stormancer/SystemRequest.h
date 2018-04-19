#pragma once

#include "stormancer/headers.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	/// System request.
	class SystemRequest
	{
	public:

#pragma region public_methods

		SystemRequest(byte msgId,pplx::task_completion_event<Packet_ptr> tce);
		virtual ~SystemRequest();
		byte operation();

#pragma endregion

#pragma region public_members

		uint16 id = 0;
		time_t lastRefresh = time(NULL);
		pplx::task_completion_event<Packet_ptr> tce;
		bool complete = false;

#pragma endregion

	private:

#pragma region private_members

		byte _msgId;

#pragma endregion
	};

	using SystemRequest_ptr = std::shared_ptr<SystemRequest>;
};
