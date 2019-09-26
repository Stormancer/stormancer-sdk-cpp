#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameFinderService.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	Internal::ReadyVerificationRequest::operator Stormancer::ReadyVerificationRequest()
	{
		Stormancer::ReadyVerificationRequest readyUpdate;
		readyUpdate.gameId = gameId;
		readyUpdate.timeout = timeout;
		readyUpdate.membersCountTotal = (int32)members.size();
		readyUpdate.membersCountReady = 0;
		for (auto it : members)
		{
			readyUpdate.members[it.first] = (Readiness)it.second;
		}
		return readyUpdate;
	}

	GameFinderService::GameFinderService(std::shared_ptr<Scene> scene)
		: _scene(scene)
		, _rpcService(scene->dependencyResolver().resolve<RpcService>())
	{
	}

	GameFinderService::~GameFinderService()
	{
		// In case the scene gets brutally destroyed without a chance to trigger onDisconnecting, make sure to notify subscribers
		onSceneDisconnecting();
	}

	void GameFinderService::initialize()
	{
		std::weak_ptr<GameFinderService> wThat = this->shared_from_this();
		_scene.lock()->addRoute("gamefinder.update", [wThat](Packetisp_ptr packet)
		{
			byte gameStateByte;
			packet->stream.read(&gameStateByte, 1);
			int32 gameState = gameStateByte;

			if (auto that = wThat.lock())
			{
				that->_currentState = (GameFinderStatus)gameState;

				auto ms = std::to_string(gameState);

				that->GameFinderStatusUpdated(that->_currentState);

				switch (that->_currentState)
				{
				case GameFinderStatus::Success:
				{
					auto dto = that->_serializer.deserializeOne<GameFinderResponseDto>(packet->stream);

					GameFinderResponse response;
					response.connectionToken = dto.connectionToken;
					response.packet = packet;

					that->GameFound(response);
					that->_currentState = GameFinderStatus::Idle;
					that->GameFinderStatusUpdated(that->_currentState);
					break;
				}
				case GameFinderStatus::Canceled:
				{
					that->_currentState = GameFinderStatus::Idle;
					that->GameFinderStatusUpdated(that->_currentState);
					break;
				}
				case GameFinderStatus::Failed:
				{
					std::string reason;
					// There may or may not be a reason string supplied with the failure notification, so check if the stream has more data
					if (packet->stream.good())
					{
						reason = that->_serializer.deserializeOne<std::string>(packet->stream);
					}
					that->FindGameRequestFailed(reason);
					that->_currentState = GameFinderStatus::Idle;
					that->GameFinderStatusUpdated(that->_currentState);
					break;
				}
				default:
				// ignore
				break;
				}
			}
		});

		_scene.lock()->addRoute("gamefinder.parameters.update", [wThat](Packetisp_ptr packet)
		{
			if (auto that = wThat.lock())
			{
				std::string provider = that->_serializer.deserializeOne<std::string>(packet->stream);
				//_onGameParametersUpdate();
			}
		});

		_scene.lock()->addRoute("gamefinder.ready.update", [wThat](Packetisp_ptr packet)
		{
			if (auto that = wThat.lock())
			{
				Internal::ReadyVerificationRequest readyUpdateTmp = that->_serializer.deserializeOne<Internal::ReadyVerificationRequest>(packet->stream);
				ReadyVerificationRequest readyUpdate = readyUpdateTmp;
				readyUpdate.membersCountReady = 0;

				for (auto it : readyUpdateTmp.members)
				{
					Readiness ready = (Readiness)it.second;
					if (ready == Readiness::Ready)
					{
						readyUpdate.membersCountReady++;
					}
				}
			}
		});
	}

	GameFinderStatus GameFinderService::currentState() const
	{
		return _currentState;
	}

	pplx::task<void> GameFinderService::findGameInternal(const std::string& provider, const StreamWriter& streamWriter)
	{
		if (currentState() != GameFinderStatus::Idle)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Already finding a game !"));
		}

		_currentState = GameFinderStatus::Searching;
		_gameFinderCTS = pplx::cancellation_token_source();

		StreamWriter streamWriter2 = [provider, streamWriter](obytestream& stream)
		{
			Serializer serializer;
			serializer.serialize(stream, provider);
			streamWriter(stream);
		};

		std::weak_ptr<GameFinderService> wThat = this->shared_from_this();
		return _rpcService.lock()->rpc("gamefinder.find", streamWriter2)
			.then([wThat](pplx::task<void> res)
		{
			// If the RPC fails (e.g. because of a disconnection), we might not have received a failed/canceled status update.
			// Make sure we go back to Idle state anyway.
			try
			{
				res.get();
			}
			catch (...)
			{
				if (auto that = wThat.lock())
				{
					if (that->_currentState != GameFinderStatus::Idle)
					{
						that->_currentState = GameFinderStatus::Idle;
						that->GameFinderStatusUpdated(that->_currentState);
					}
				}
				throw;
			}
			return res;
		});
	}

	pplx::task<void> GameFinderService::findGame(const std::string &provider, const StreamWriter& streamWriter)
	{
		return findGameInternal(provider, streamWriter);
	}

	void GameFinderService::resolve(bool acceptGame)
	{
		auto scene = _scene.lock();
		scene->send("gamefinder.ready.resolve", [=](obytestream& stream)
		{
			stream << acceptGame;
		}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED);
	}

	void GameFinderService::cancel()
	{
		if (currentState() != GameFinderStatus::Idle)
		{
			auto scene = _scene.lock();
			_gameFinderCTS.cancel();
			scene->send("gamefinder.cancel", [](obytestream&) {}, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::RELIABLE_ORDERED);
		}
	}

	void GameFinderService::onSceneDisconnecting()
	{
		if (_currentState != GameFinderStatus::Idle &&
			_currentState != GameFinderStatus::Canceled &&
			_currentState != GameFinderStatus::Failed &&
			_currentState != GameFinderStatus::Success)
		{
			_currentState = GameFinderStatus::Failed;
			GameFinderStatusUpdated(_currentState);
		}
	}
}