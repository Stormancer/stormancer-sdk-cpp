#include "stormancer/headers.h"
#include "stormancer/RPC/RpcService.h"
#include "GameSessionService.h"

namespace Stormancer
{


	GameSessionService::GameSessionService(Scene_ptr scene) :
		_onShutdownReceived([]() {})
	{
		_scene = scene;

		_logger = scene->dependencyResolver()->resolve<ILogger>();

		_scene->addRoute("gameSession.shutdown", [this](Packetisp_ptr packet) {
			if (this->_onShutdownReceived)
			{
				this->_onShutdownReceived();
			}
		});
		_scene->addRoute("player.update", [this](Packetisp_ptr packet) {
			auto update = packet->readObject<Stormancer::PlayerUpdate>();
			SessionPlayer player(update.UserId, (PlayerStatus)update.Status);

			auto end = this->_users.end();
			auto it = std::find_if(this->_users.begin(), end, [player](SessionPlayer p) { return p.PlayerId == player.PlayerId; });
			if (it == end)
			{
				this->_users.push_back(player);
			}
			else
			{
				*it = player;
			}
			this->_onConnectedPlayersChanged(SessionPlayerUpdateArg(player, update.Data));
		});

		_scene->addRoute("player.p2ptoken", [this](Packetisp_ptr packet) {
			auto p2pToken = packet->readObject<std::string>();
			if (p2pToken.empty()) //host
			{
				this->_onRoleReceived("HOST");
				_tunnel = _scene->registerP2PServer(GAMESESSION_P2P_SERVER_ID);
			}
			else //client
			{
				_scene->openP2PConnection(p2pToken).then([this](std::shared_ptr<Stormancer::P2PScenePeer> p2pPeer) {
					this->_onRoleReceived("CLIENT");

					if (_onConnectionOpened)
					{
						_onConnectionOpened(p2pPeer);
					}
					if (shouldEstablishTunnel)
					{
						return p2pPeer->openP2PTunnel(GAMESESSION_P2P_SERVER_ID).then([this](std::shared_ptr<Stormancer::P2PTunnel> guestTunnel)
						{
							if (_onTunnelOpened)
							{
								_onTunnelOpened(guestTunnel);
							}
						});
					}
					else
					{
						return pplx::task_from_result();
					}
				}).then([this](pplx::task<void> t) {
					try
					{
						t.get();
					}
					catch (const std::exception& ex)
					{
						_onConnectionFailure(ex.what());
						_logger->log(ex);
					}
				});
			}
		});
	}

	pplx::task<std::string> GameSessionService::waitServerReady(pplx::cancellation_token token)
	{
		return pplx::create_task(_waitServerTce, pplx::task_options(token));
	}

	std::vector<SessionPlayer> GameSessionService::GetConnectedPlayers()
	{
		return this->_users;
	}

	void GameSessionService::unsubscribeConnectedPlayersChanged(Action<SessionPlayerUpdateArg>::TIterator handle)
	{
		this->_onConnectedPlayersChanged.erase(handle);
	}

	std::function<void()> Stormancer::GameSessionService::OnConnectedPlayerChanged(std::function<void(SessionPlayer, std::string)> callback)
	{
		auto iterator = this->_onConnectedPlayersChanged.push_back([callback](SessionPlayerUpdateArg args) {callback(args.sessionPlayer, args.data); });
		return [iterator, this]() {
			unsubscribeConnectedPlayersChanged(iterator);
		};
	}

	void GameSessionService::unsubscribeRoleReceived(Action < std::string > ::TIterator handle)
	{
		this->_onRoleReceived.erase(handle);
	}

	std::function<void()> Stormancer::GameSessionService::OnRoleReceived(std::function<void(std::string)> callback)
	{
		auto iterator = this->_onRoleReceived.push_back([callback](std::string role) {callback(role); });
		return [iterator, this]() {
			unsubscribeRoleReceived(iterator);
		};
	}

	void GameSessionService::OnTunnelOpened(std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)> callback)
	{
		_onTunnelOpened = callback;
	}

	void GameSessionService::OnP2PConnected(std::function<void(std::shared_ptr<Stormancer::P2PScenePeer>)> callback)
	{
		_onConnectionOpened = callback;
	}

	void GameSessionService::OnShutdownReceived(std::function<void(void)> callback)
	{
		_onShutdownReceived = callback;
	}

	void GameSessionService::OnConnectionFailure(std::function<void(std::string)> callback)
	{
		_onConnectionFailure += callback;
	}

	pplx::task<void> GameSessionService::connect()
	{
		return _scene->connect();
	}

	pplx::task<void> GameSessionService::reset()
	{
		auto rpc = _scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpcWriter("gamesession.reset", [](obytestream*) {});
	}

	pplx::task<void> GameSessionService::disconnect()
	{
		if (_scene)
		{
			return _scene->disconnect();
		}
		else
		{
			return pplx::task_from_result();
		}
	}

	void GameSessionService::ready(std::string data)
	{
		_scene->send("player.ready", [data](obytestream* stream) {
			msgpack::pack(stream, data);
		});
	}

	GameSessionManager::GameSessionManager(Client* client) :client(client)
	{

	}

	void GameSessionManager::setToken(std::string newToken)
	{
		this->currentGameSessionId = "";
		this->token = newToken;
	}

	pplx::task<Stormancer::Scene_ptr> GameSessionManager::getCurrentGameSession()
	{
		if (this->currentGameSessionId.empty())
		{
			return this->client->getConnectedScene(this->currentGameSessionId);
		}
		else
		{
			return this->client->getPrivateScene(this->token).then([this](pplx::task<Scene_ptr> t) {
				auto scene = t.get();
				this->currentGameSessionId = scene->id();
				return scene;
			});
		}
	}
}
