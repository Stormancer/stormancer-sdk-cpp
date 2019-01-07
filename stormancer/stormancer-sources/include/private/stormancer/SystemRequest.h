#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Packet.h"

namespace Stormancer
{
	/// System request.
	class SystemRequest
	{
	public:

#pragma region public_methods

		SystemRequest(byte msgId,pplx::task_completion_event<Packet_ptr> tce, pplx::cancellation_token ct);
		virtual ~SystemRequest();
		byte operation();

#pragma endregion

#pragma region public_members

		uint16 id = 0;
		time_t lastRefresh = time(NULL);
		pplx::task_completion_event<Packet_ptr> tce;
		bool complete = false;

#pragma endregion
		pplx::cancellation_token ct;
		pplx::cancellation_token_registration ct_registration;
	private:

#pragma region private_members

		byte _msgId;

#pragma endregion
	};

	using SystemRequest_ptr = std::shared_ptr<SystemRequest>;
};
