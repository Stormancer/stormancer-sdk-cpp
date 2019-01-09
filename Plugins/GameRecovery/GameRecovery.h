#pragma once
#include "Core/ClientAPI.h"
#include "GameRecovery/GameRecoveryService.h"

namespace Stormancer
{
	struct RecoverableGame
	{
		std::string gameId;
		std::string userData;
	};

	class GameRecovery :public ClientAPI<GameRecovery>
	{
	public:
		GameRecovery(std::weak_ptr<AuthenticationService> auth);

		pplx::task<std::shared_ptr<RecoverableGame>> getCurrent();

		pplx::task<void> cancelCurrent();


	private:
		pplx::task<std::shared_ptr<GameRecoveryService>> getGRService();
	};
}