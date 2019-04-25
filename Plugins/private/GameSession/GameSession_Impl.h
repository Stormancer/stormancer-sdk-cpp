#pragma once

#include "stormancer/Event.h"
#include "GameSession/GameSessionModels.h"
#include "GameSession/GameSession.h"
#include "stormancer/ConnectionState.h"

namespace Stormancer
{
	//Forward declare
	class IClient;
	struct GameSessionContainer;
	
	class GameSession_Impl : public std::enable_shared_from_this<GameSession_Impl>, public GameSession
	{
	public:
		GameSession_Impl(std::weak_ptr<IClient> client);

		pplx::task<GameSessionConnectionParameters> ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		pplx::task<void> SetPlayerReady(std::string data, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		pplx::task<Packetisp_ptr> PostResult(const StreamWriter& streamWriter, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		pplx::task<std::string> GetUserFromBearerToken(std::string token) override;
		pplx::task<void> DisconectFromGameSession() override;

		Event<void>::Subscription subscibeOnAllPlayerReady(std::function<void()> callback) override;
		Event<GameSessionConnectionParameters>::Subscription subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback) override;
		Event<GameSessionConnectionParameters>::Subscription subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback) override;
		Event<ConnectionState>::Subscription subscribeOnGameSessionConnectionChange(std::function<void(ConnectionState)> callback) override;
		Event<void>::Subscription subscribeOnShutdownRecieved(std::function<void()> callback) override;
	private:

		//Events
		Event<void> OnAllPlayerReady;
		Event<GameSessionConnectionParameters> OnRoleRecieved;
		Event<GameSessionConnectionParameters> OnTunnelOpened;
		Event<GameSessionResult> OnPostedResultsReceived;
		Event<ConnectionState> OnGameSessionConnectionChange;
		Event<void> _onShutdownReceived;

		std::string _mapName;
		std::weak_ptr<IClient> _wClient;
		pplx::task<std::shared_ptr<GameSessionContainer>> _currentGameSession;

		pplx::task<std::shared_ptr<GameSessionContainer>> connectToGameSessionImpl(std::string token, pplx::cancellation_token ct);
		pplx::task<std::shared_ptr<GameSessionContainer>> getCurrentGameSession(pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<std::string> P2PTokenRequest(std::shared_ptr<GameSessionContainer> gameSessionContainer, pplx::cancellation_token ct);

		std::mutex _lock;
	};
}