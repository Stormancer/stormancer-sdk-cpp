#include "GameSession/GameSessionService.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	GameSessionService::GameSessionService(std::weak_ptr<Scene> scene) :
		_scene(scene),
		_logger(scene.lock()->dependencyResolver()->resolve<ILogger>())
	{
	}

	GameSessionService::~GameSessionService()
	{
		_logger->log("GameSessionService destroyed");
	}

	void GameSessionService::initialize()
	{
		_disconnectionCts = pplx::cancellation_token_source();
		std::weak_ptr<GameSessionService> wThat = this->shared_from_this();

		_scene.lock()->addRoute("gameSession.shutdown", [wThat](Packetisp_ptr packet)
		{
			auto that = wThat.lock();
			if (that)
			{
				that->onShutdownReceived();
			}
		});

		_scene.lock()->addRoute("player.update", [wThat](Packetisp_ptr packet)
		{
			auto that = wThat.lock();
			if (that)
			{
				auto update = packet->readObject<Stormancer::PlayerUpdate>();
				SessionPlayer player(update.userId, (PlayerStatus)update.status, update.isHost);

				auto end = that->_users.end();
				auto it = std::find_if(that->_users.begin(), end, [player](SessionPlayer p) { return p.playerId == player.playerId; });
				if (it == end)
				{
					that->_users.push_back(player);
				}
				else
				{
					*it = player;
				}
				that->onPlayerChanged(player, update.data);
			}
		});

		_scene.lock()->addRoute("players.allReady", [wThat](Packetisp_ptr packet) {
			auto that = wThat.lock();
			if (that)
			{
				that->onAllPlayerReady();
			}
		});
	}

	pplx::task<void> GameSessionService::initializeTunnel(std::string p2pToken, pplx::cancellation_token ct)
	{
		ct = linkTokenToDisconnection(ct);
		auto scene = _scene.lock();
		if (!scene)
		{
			_logger->log(LogLevel::Error, "gamession.p2ptoken", "scene deleted");
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"), ct);
		}

		_logger->log(LogLevel::Trace, "gamession.p2ptoken", "recieved p2p token");
		if (_receivedP2PToken)
		{
			return pplx::task_from_result(pplx::task_options(ct));
		}

		_receivedP2PToken = true;
		_waitServerTce.set();
		if (p2pToken.empty()) //host
		{
			_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received empty p2p token: I'm the host.");
			onRoleReceived("HOST");
			_waitServerTce.set();
			_tunnel = scene->registerP2PServer(GAMESESSION_P2P_SERVER_ID);
			return pplx::task_from_result(pplx::task_options(ct));
		}
		else //client
		{
			_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received valid p2p token: I'm a client.");

			std::weak_ptr<GameSessionService> wThat = this->shared_from_this();
			return scene->openP2PConnection(p2pToken, ct).then([wThat, ct](std::shared_ptr<Stormancer::IP2PScenePeer> p2pPeer) {
				auto that = wThat.lock();
				if (that)
				{
					that->onRoleReceived("CLIENT");
					if (that->_onConnectionOpened)
					{
						that->_onConnectionOpened(p2pPeer);
					}

					if (that->shouldEstablishTunnel)
					{
						return p2pPeer->openP2PTunnel(GAMESESSION_P2P_SERVER_ID, ct).then([wThat](std::shared_ptr<Stormancer::P2PTunnel> guestTunnel)
						{
							auto that = wThat.lock();
							if (that)
							{
								that->_tunnel = guestTunnel;
								that->onTunnelOpened(guestTunnel);
							}
						}, ct);
					}
					else
					{
						return pplx::task_from_exception<void>(std::runtime_error("Service destroyed"), ct);
					}
				}
				else
				{
					return pplx::task_from_exception<void>(std::runtime_error("Service destroyed"));
				}
			}, ct).then([wThat](pplx::task<void> t) {
				auto that = wThat.lock();
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					if (that)
					{
						that->_onConnectionFailure(ex.what());
						that->_logger->log(ex);
					}
					throw;
				}
			}, ct);
		}
	}

	pplx::task<void> GameSessionService::waitServerReady(pplx::cancellation_token token)
	{
		token = linkTokenToDisconnection(token);
		return pplx::create_task(_waitServerTce, pplx::task_options(token));
	}

	std::vector<SessionPlayer> GameSessionService::getConnectedPlayers()
	{
		return this->_users;
	}

	pplx::cancellation_token GameSessionService::linkTokenToDisconnection(pplx::cancellation_token tokenToLink)
	{
		if (tokenToLink.is_cancelable())
		{
			auto tokens = { tokenToLink, _disconnectionCts.get_token() };
			return pplx::cancellation_token_source::create_linked_source(tokens.begin(), tokens.end()).get_token();
		}
		else
		{
			return _disconnectionCts.get_token();
		}
	}

	void GameSessionService::onP2PConnected(std::function<void(std::shared_ptr<Stormancer::IP2PScenePeer>)> callback)
	{
		_onConnectionOpened = callback;
	}

	void GameSessionService::onConnectionFailure(std::function<void(std::string)> callback)
	{
		_onConnectionFailure += callback;
	}

	std::weak_ptr<Scene> GameSessionService::getScene()
	{
		return _scene;
	}

	pplx::task<std::string> GameSessionService::getUserFromBearerToken(std::string token)
	{
		if (auto scene = _scene.lock())
		{
			auto rpc = scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<std::string, std::string>("GameSession.GetUserFromBearerToken", token);
		}
		else
		{
			throw pplx::task_from_exception<std::string>(std::runtime_error("Scene destroyed"));
		}
	}

	pplx::task<std::string> GameSessionService::p2pTokenRequest(pplx::cancellation_token ct)
	{
		if (auto scene = _scene.lock())
		{
			ct = linkTokenToDisconnection(ct);
			auto rpc = scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<std::string, int>("GameSession.GetP2PToken", ct, 1);
		}
		else
		{
			throw pplx::task_from_exception<std::string>(std::runtime_error("Scene destroyed"), ct);
		}
	}

	pplx::task<void> GameSessionService::reset(pplx::cancellation_token ct)
	{
		ct = linkTokenToDisconnection(ct);
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene deleted"), ct);
		}

		auto rpc = scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpc("gamesession.reset", ct);
	}

	pplx::task<void> GameSessionService::disconnect()
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_result();
		}

		return scene->disconnect();
	}

	void GameSessionService::onDisconnecting()
	{
		_tunnel = nullptr;
		_users.clear();
		_disconnectionCts.cancel();
	}

	void GameSessionService::ready(std::string data)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			_logger->log(LogLevel::Error, "category", "scene deleted");
			return;
		}

		scene->send("player.ready", [data](obytestream& stream)
		{
			msgpack::pack(stream, data);
		});
	}

	pplx::task<Packetisp_ptr> GameSessionService::sendGameResults(const StreamWriter& streamWriter, pplx::cancellation_token ct)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<Packetisp_ptr>(std::runtime_error("Scene deleted"));
		}

		auto rpc = scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpc<Packetisp_ptr>("gamesession.postresults", ct, streamWriter);
	}
}
