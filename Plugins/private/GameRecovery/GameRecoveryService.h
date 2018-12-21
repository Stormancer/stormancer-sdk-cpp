#pragma once
#include "stormancer/RPC/service.h"



namespace Stormancer
{
	/// Forward declare
	struct RecoverableGameDto;

	class GameRecoveryService
	{
	public:

#pragma region public_methods

		GameRecoveryService(std::shared_ptr<Scene> scene);
		
		/// Tries to obtain the current recoverable game of the player.
		pplx::task<std::shared_ptr<RecoverableGameDto>> getCurrent();

		pplx::task<void> cancelCurrent();

#pragma endregion

	private:

#pragma region
		std::weak_ptr<Scene> _scene;
		std::weak_ptr<RpcService> _rpcService;
#pragma endregion
	};
}
