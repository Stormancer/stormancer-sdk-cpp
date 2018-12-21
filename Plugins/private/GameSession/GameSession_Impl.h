#pragma once

#include "stormancer/Event.h"
#include "GameSession/GameSessionModels.h"
#include "GameSession/GameSession.h"

namespace Stormancer
{
	//Forward declare
	class IClient;
	struct GameSessionContainer;
	
	class GameSession_Impl : public std::enable_shared_from_this<GameSession_Impl>, public GameSession
	{
	public:
		GameSession_Impl(std::weak_ptr<IClient> client);

		pplx::task<GameSessionConnectionParameters> ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> SetPlayerReady(std::string data, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<GameSessionResult> PostResult(EndGameDto gameSessioResult, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> DisconectFromGameSession();		

		Event<void>::Subscription subscibeOnAllPlayerReady(std::function<void()> callback) override;
		Event<GameSessionConnectionParameters>::Subscription subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback) override;
		Event<GameSessionConnectionParameters>::Subscription subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback) override;
	private:

		//Events
		Event<void> OnAllPlayerReady;
		Event<GameSessionConnectionParameters> OnRoleRecieved;
		Event<GameSessionConnectionParameters> OnTunnelOpened;

		std::string _mapName;
		std::weak_ptr<IClient> _wClient;
		pplx::task<std::shared_ptr<GameSessionContainer>> _currentGameSession;
		pplx::task_completion_event<GameSessionConnectionParameters> _gameSessionNegotiationTce;

		pplx::task<std::shared_ptr<GameSessionContainer>> connectToGameSessionImpl(std::string token, pplx::cancellation_token ct);
		pplx::task<std::shared_ptr<GameSessionContainer>> getCurrentGameSession(pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<std::string> P2PTokenRequest(std::shared_ptr<GameSessionContainer> gameSessionContainer, pplx::cancellation_token ct);

		std::mutex _lock;
	};
}