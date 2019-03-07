#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Packet.h"
#include <string>

namespace Stormancer
{
	class STORMANCER_DLL_API RpcRequest
	{
	public:

		RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer, std::string route);
		~RpcRequest();

		uint16 id = 0;
		rxcpp::subscriber<Packetisp_ptr> observer;
		pplx::task_completion_event<void> waitingForDataTce;
		bool hasCompleted = false;
		std::string route;
	};

	using RpcRequest_ptr = std::shared_ptr<RpcRequest>;
};
