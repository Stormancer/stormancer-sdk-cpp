#pragma once

#include "Users/ClientAPI.hpp"
#include "GameRecovery/GameRecovery.h"

namespace Stormancer
{
	namespace Users
	{
		class UsersApi;
	}
	struct RecoverableGame;
	class GameRecoveryService;

	class GameRecovery_Impl : public ClientAPI<GameRecovery_Impl>, public GameRecovery
	{
	public:
		GameRecovery_Impl(std::weak_ptr<Users::UsersApi> users);
		
		pplx::task<std::shared_ptr<RecoverableGame>> getCurrent() override;

		pplx::task<void> cancelCurrent() override;

	private:
		pplx::task<std::shared_ptr<GameRecoveryService>> getGRService();
	};
}