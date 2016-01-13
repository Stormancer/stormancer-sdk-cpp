#pragma once
#include "headers.h"
#include "IObservable.h"
#include "RpcRequestContext.h"
#include "Scene.h"

namespace Stormancer
{
	class IRpcService
	{
	public:
		virtual IObservable<Packetisp_ptr>* rpc(const char* route, std::function<void(bytestream*)> writer_ptr, PacketPriority priority) = 0;
		virtual void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered) = 0;
		virtual uint16 pendingRequests() = 0;
		virtual void cancelAll(const char* reason) = 0;
	};
};
