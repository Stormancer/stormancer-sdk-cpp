#include "stormancer/headers.h"
#include "MatchmakingService.h"

namespace Stormancer
{
	MatchmakingService::MatchmakingService(std::shared_ptr<Scene> scene)
		: _scene(scene)
	{
		_rpcService = scene->dependencyResolver()->resolve<RpcService>();
		_logger = scene->dependencyResolver()->resolve<ILogger>();

		scene->addRoute("match.update", [this](Packetisp_ptr packet) {
			byte matchStateByte;
			packet->stream->read((char*)&matchStateByte, 1);
			int32 matchState = matchStateByte;

			_matchState = (MatchState)matchState;

			auto ms = std::to_string(matchState);

			if (_onMatchUpdate)
			{
				_onMatchUpdate(_matchState);
			}

			if (_matchState == MatchState::Success)
			{
				Serializer serializer;
				MatchmakingResponse mmres = serializer.deserializeOne<MatchmakingResponse>(packet->stream);

				if (_onMatchFound)
				{
					_onMatchFound(mmres);
				}
			}
		});

		scene->addRoute("match.parameters.update", [](Packetisp_ptr packet) {
			Serializer serializer;	
			std::string provider = serializer.deserializeOne<std::string>(packet->stream);
			//_onMatchParametersUpdate();
		});

		scene->addRoute("match.ready.update", [this](Packetisp_ptr packet) {
			Serializer serializer;
			ReadyVerificationRequest readyUpdate = serializer.deserializeOne<ReadyVerificationRequest>(packet->stream);
			readyUpdate.membersCountReady = 0;

			for (auto it : readyUpdate.members)
			{
				if (it.second == Readiness::Ready)
				{
					readyUpdate.membersCountReady++;
				}
			}

			if (_onMatchReadyUpdate)
			{
				_onMatchReadyUpdate(readyUpdate);
			}
		});
	}

	MatchmakingService::~MatchmakingService()
	{
		this->cancel();
	}

	void MatchmakingService::onMatchUpdate(std::function<void(MatchState)> callback)
	{
		_onMatchUpdate = callback;
	}

	void MatchmakingService::onMatchReadyUpdate(std::function<void(ReadyVerificationRequest)> callback)
	{
		_onMatchReadyUpdate = callback;
	}

	void MatchmakingService::onMatchFound(std::function<void(MatchmakingResponse)> callback)
	{
		_onMatchFound = callback;
	}

	MatchState MatchmakingService::matchState() const
	{
		return _matchState;
	}

	pplx::task<void> MatchmakingService::findMatch(const std::string &provider, const MatchmakingRequest &mmRequest)
	{
		if (_isMatching)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Already matching !"));
		}

		_isMatching = true;
		_matchmakingCTS = pplx::cancellation_token_source();
		auto matchmakingToken = _matchmakingCTS.get_token();

		return _rpcService->rpc<void>("match.find", provider, mmRequest)
			.then([this, matchmakingToken](pplx::task<void> res)
		{
			if (matchmakingToken.is_canceled())
			{					
				return pplx::task_from_exception<void>(std::runtime_error("Matchmaking canceled"));
			}
			else 
			{
				_isMatching = false;
				return res;
			}
		});
	}

	void MatchmakingService::resolve(bool acceptMatch)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			_logger->log(LogLevel::Error, "MatchmakingService", "scene deleted");
			return;
		}

		scene->send("match.ready.resolve", [acceptMatch](obytestream* stream) {
			(*stream) << acceptMatch;
		}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED);
	}

	void MatchmakingService::cancel()
	{
		if (_isMatching)
		{
			_matchmakingCTS.cancel();
			_isMatching = false;

			auto scene = _scene.lock();
			if (!scene)
			{
				_logger->log(LogLevel::Error, "MatchmakingService", "scene deleted");
				return;
			}

			scene->send("match.cancel", [](obytestream*) {}, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::RELIABLE_ORDERED);
		}
	}
};
