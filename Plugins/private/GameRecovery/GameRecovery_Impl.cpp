#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameRecovery_Impl.h"
#include "GameRecovery/GameRecoveryModels.h"
#include "GameRecoveryService.h"

namespace Stormancer
{
	GameRecovery_Impl::GameRecovery_Impl(std::weak_ptr<Users::UsersApi> users)
		: ClientAPI(users, "stormancer.gameRecovery")
	{
	}

	pplx::task<std::shared_ptr<RecoverableGame>> GameRecovery_Impl::getCurrent()
	{
		return getGRService()
			.then([](std::shared_ptr<GameRecoveryService> gr) {
			return gr->getCurrent();
		})
			.then([](std::shared_ptr<RecoverableGameDto> dto) {

			if (dto)
			{
				auto result = std::make_shared<RecoverableGame>();
				result->gameId = dto->gameId;
				result->userData = dto->userData;
				return result;
			}
			else
			{
				return (std::shared_ptr<RecoverableGame>)nullptr;
			}
		});
	}

	pplx::task<void> GameRecovery_Impl::cancelCurrent()
	{
		return getGRService().then([](std::shared_ptr<GameRecoveryService> gr) { return gr->cancelCurrent(); });
	}

	pplx::task<std::shared_ptr<GameRecoveryService>> GameRecovery_Impl::getGRService()
	{
		return this->getService();
	}
}
