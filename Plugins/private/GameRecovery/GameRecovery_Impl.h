#pragma once

#include "stormancer/ClientAPI.h"
#include "GameRecovery/GameRecovery.h"

namespace Stormancer
{
	class AuthenticationService;
	struct RecoverableGame;
	class GameRecoveryService;

	class GameRecovery_Impl : public ClientAPI<GameRecovery_Impl>, public GameRecovery
	{
	public:
		GameRecovery_Impl(std::weak_ptr<AuthenticationService> auth);
		
		pplx::task<std::shared_ptr<RecoverableGame>> getCurrent() override;

		pplx::task<void> cancelCurrent() override;

	private:
		pplx::task<std::shared_ptr<GameRecoveryService>> getGRService();
	};
}