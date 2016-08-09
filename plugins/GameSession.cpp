#include "GameSession.h"
#include "RpcHelpers.h"

namespace Stormancer
{
	GameSessionService::GameSessionService(Scene* scene)
	{
		_scene = scene;
		scene->addRoute("server.started", [this](auto packet) {
			auto serverInfos = readObject<GameServerInformations>(packet);
			this->_waitServerTce.set(serverInfos);
		});
		scene->addRoute("player.joined", [this](auto packet) {
			auto userId = readObject<std::string>(packet);
			auto end = this->_users.end();
			auto it = std::find(this->_users.begin(), end, userId);
			if (it == end)
			{
				this->_users.push_back(userId);
				this->_onConnectedPlayersChanged();
			}
		});
		scene->addRoute("player.left", [this](auto packet) {
			auto userId = readObject<std::string>(packet);
			auto end = this->_users.end();
			auto it = std::find(this->_users.begin(), end, userId);
			if (it != end)
			{
				this->_users.erase(it);
				this->_onConnectedPlayersChanged();
			}
		});

	}

	pplx::task<std::shared_ptr<Result<GameServerInformations>>> GameSessionService::WaitServerReady(pplx::cancellation_token token)
	{
		return pplx::create_task(_waitServerTce, pplx::task_options(token)).then([](pplx::task<GameServerInformations> t) {

			std::shared_ptr<Stormancer::Result<GameServerInformations>> result(new Stormancer::Result<GameServerInformations>());
			try
			{
				result->set(t.get());
			}
			catch (const std::exception& ex)
			{
				result->setError(1, ex.what());

			}
			return result;

		});
	}

	std::vector<std::string> GameSessionService::getConnectedPlayers()
	{
		return this->_users;
	}

	void GameSessionService::unsubscribeConnectedPlayersChanged(Action<>::TIterator handle)
	{
		this->_onConnectedPlayersChanged.erase(handle);
	}
	Action<>::TIterator GameSessionService::onConnectedPlayersChanged(std::function<void()> callback)
	{
		return this->_onConnectedPlayersChanged.push_back(callback);
	}

}