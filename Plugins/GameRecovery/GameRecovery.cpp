#include "stormancer/headers.h"

#include "GameRecovery/GameRecovery.h"


namespace Stormancer
{
	GameRecovery::GameRecovery(std::weak_ptr<AuthenticationService> auth) :ClientAPI(auth) {}

	pplx::task<std::shared_ptr<RecoverableGame>> GameRecovery::getCurrent()
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

	pplx::task<void> GameRecovery::cancelCurrent()
	{
		return getGRService().then([](std::shared_ptr<GameRecoveryService> gr) {return gr->cancelCurrent(); });
	}
	pplx::task<std::shared_ptr<GameRecoveryService>> GameRecovery::getGRService()
	{
		return this->getService<GameRecoveryService>("stormancer.gameRecovery");
	}
}