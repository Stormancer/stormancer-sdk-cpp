#include "GameRecovery/GameRecoveryService.h"

namespace Stormancer
{
	GameRecoveryService::GameRecoveryService(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		if (scene)
		{
			_rpcService = scene->dependencyResolver()->resolve<RpcService>();
		}
	}

	pplx::task<std::shared_ptr<RecoverableGameDto>> GameRecoveryService::getCurrent()
	{
		return _rpcService->rpc<std::shared_ptr<RecoverableGameDto>>("gamerecovery.getcurrent");
	}

	pplx::task<void> GameRecoveryService::cancelCurrent()
	{
		return _rpcService->rpc<void>("gamerecovery.cancelcurrent");
	}

}
