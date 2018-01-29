#include "stdafx.h"
#include <RPC/RpcService.h>
#include "GameSessionService.h"

namespace Stormancer
{
	GameSessionService::GameSessionService(Scene_ptr scene)
	{
		_scene = scene;

		_scene->addRoute("server.started", [this](Packetisp_ptr packet) {
			auto serverInfos = packet->readObject<GameServerInformations>();
			this->_waitServerTce.set(serverInfos);
		});

		_scene->addRoute("player.update", [this](Packetisp_ptr packet) {
			auto update = packet->readObject<PlayerUpdate>();
			SessionPlayer player{ update.UserId, (PlayerStatus)update.Status };

			auto end = _users.end();
			auto it = std::find_if(_users.begin(), end, [player](SessionPlayer p) { return p.PlayerId == player.PlayerId; });
			if (it == end)
			{
				_users.push_back(player);
			}
			else
			{
				*it = player;
			}
			_onConnectedPlayersChanged(player);
		});
	}

	pplx::task<GameServerInformations> GameSessionService::waitServerReady(pplx::cancellation_token token)
	{
		return pplx::create_task(_waitServerTce, pplx::task_options(token));
	}

	std::vector<SessionPlayer> GameSessionService::getConnectedPlayers()
	{
		return _users;
	}

	void GameSessionService::unsubscribeConnectedPlayersChanged(Action<SessionPlayer>::TIterator handle)
	{
		_onConnectedPlayersChanged.erase(handle);
	}

	std::function<void()> GameSessionService::onConnectedPlayerChanged(std::function<void(SessionPlayer)> callback)
	{
		auto iterator = _onConnectedPlayersChanged.push_back(callback);
		return [iterator, this]() {
			unsubscribeConnectedPlayersChanged(iterator);
		};
	}

	pplx::task<void> GameSessionService::connect()
	{
		return _scene->connect();
	}

	pplx::task<void> GameSessionService::reset()
	{
		auto rpc = _scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpcWriter("gamesession.reset", Writer());
	}

	void GameSessionService::ready()
	{
		_scene->send("gamesession.playerready", Writer());
	}
}
