#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameRecoveryService.h"
#include "GameRecoveryModels.h"

namespace Stormancer
{
	GameRecoveryService::GameRecoveryService(std::shared_ptr<Scene> scene)
		: _scene(scene)
		, _rpcService(scene->dependencyResolver()->resolve<RpcService>())
	{
	}

	pplx::task<std::shared_ptr<RecoverableGameDto>> GameRecoveryService::getCurrent()
	{
		return _rpcService.lock()->rpc<std::shared_ptr<RecoverableGameDto>>("gamerecovery.getcurrent");
	}

	pplx::task<void> GameRecoveryService::cancelCurrent()
	{
		return _rpcService.lock()->rpc<void>("gamerecovery.cancelcurrent");
	}

}
