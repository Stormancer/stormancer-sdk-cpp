#pragma once
#include "stormancer/headers.h"
#include "stormancer/RPC/RpcService.h"

namespace Stormancer
{
	struct RecoverableGameDto
	{
		std::string gameId;
		std::string userData;
		MSGPACK_DEFINE(gameId,userData)
	};

	class GameRecoveryService
	{
	public:

#pragma region public_methods

		GameRecoveryService(Scene* scene);
		
		/// Tries to obtain the current recoverable game of the player.
		pplx::task<std::shared_ptr<RecoverableGameDto>> getCurrent();

		pplx::task<void> cancelCurrent();

#pragma endregion

	private:

#pragma region

		Scene* _scene = nullptr;
		std::shared_ptr<RpcService> _rpcService;

#pragma endregion
	};
}