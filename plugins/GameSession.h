#pragma once
#include "headers.h"
#include <stormancer.h>

namespace Stormancer
{
	struct GameServerInformations
	{
	public:
		std::string Ip;
		int Port;

		MSGPACK_DEFINE(Ip, Port);
	};
	class GameSessionService
	{
	public:
		GameSessionService(Stormancer::Scene* scene);

		pplx::task<std::shared_ptr<Result<GameServerInformations>>> WaitServerReady(pplx::cancellation_token);

		std::vector<std::string> getConnectedPlayers();

		void unsubscribeConnectedPlayersChanged(Action<>::TIterator handle);
		Action<>::TIterator onConnectedPlayersChanged(std::function<void()> callback);

	private:
		Action<> _onConnectedPlayersChanged;
		Scene* _scene;
		std::vector<std::string> _users;
		pplx::task_completion_event<GameServerInformations> _waitServerTce;
	};
}