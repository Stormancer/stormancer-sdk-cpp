#pragma once
#include "headers.h"
#include "Scene.h"

namespace Stormancer
{
	class RpcService
	{
	public:
		struct PluginRequest
		{
			rxcpp::observer<Packet<IScenePeer>*> observer;
			int receivedMsg;
			pplx::task_completion_event<void> tce;
		};

	public:
		RpcService(Scene* scene);
		virtual ~RpcService();

		// TODO

	private:
		uint16 _currentRequestId = 0;
		std::mutex mutexForId;
		std::map<uint16, Request*> _pendingRequests;
		Scene* _scene;
	};
};
