#pragma once

#include "stormancer/headers.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	class STORMANCER_DLL_API RpcRequest
	{
	public:

		RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer);
		~RpcRequest();

		uint16 id = 0;
		rxcpp::subscriber<Packetisp_ptr> observer;
		pplx::task_completion_event<void> tce;
		bool hasCompleted = false;
	};

	using RpcRequest_ptr = std::shared_ptr<RpcRequest>;
};
