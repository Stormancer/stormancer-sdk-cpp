#include "stormancer/headers.h"
#include "MatchmakingService.h"

namespace Stormancer
{
	Internal::ReadyVerificationRequest::operator Stormancer::ReadyVerificationRequest()
	{
		Stormancer::ReadyVerificationRequest readyUpdate;
		readyUpdate.matchId = matchId;
		readyUpdate.timeout = timeout;
		readyUpdate.membersCountTotal = (int32)members.size();
		readyUpdate.membersCountReady = 0;
		for (auto it : members)
		{
			readyUpdate.members[it.first] = (Readiness)it.second;
		}
		return readyUpdate;
	}

	MatchmakingService::MatchmakingService(Scene_ptr scene)
		: _scene(scene)
		, _rpcService(scene->dependencyResolver()->resolve<RpcService>())
	{
		_scene->addRoute("match.update", [=](Packetisp_ptr packet) {
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

		_scene->addRoute("match.parameters.update", [=](Packetisp_ptr packet) {
			Serializer serializer;
			std::string provider = serializer.deserializeOne<std::string>(packet->stream);

			//_onMatchParametersUpdate();
		});

		_scene->addRoute("match.ready.update", [=](Packetisp_ptr packet) {
			Serializer serializer;
			Internal::ReadyVerificationRequest readyUpdateTmp = serializer.deserializeOne<Internal::ReadyVerificationRequest>(packet->stream);
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

			if (_onMatchReadyUpdate)
			{
				_onMatchReadyUpdate(readyUpdate);
			}
		});
	}

	MatchmakingService::~MatchmakingService()
	{		
		if (_hasSubscription)
		{
			_matchmakingSubscription.unsubscribe();
			_hasSubscription = false;
		}
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

	pplx::task<void> MatchmakingService::findMatch(std::string provider)
	{
		pplx::task_completion_event<void> tce;

		_isMatching = true;

		rxcpp::observable<Packetisp_ptr> observable = _rpcService->rpc_observable("match.find", [=](obytestream* stream) {
			_serializer.serialize(stream, provider);
		}, PacketPriority::MEDIUM_PRIORITY);

		_matchmakingSubscription = observable.subscribe([=](Packetisp_ptr packet) {
			// On next
			_isMatching = false;
			if (_hasSubscription)
			{
				_matchmakingSubscription.unsubscribe();
				_hasSubscription = false;
			}
			tce.set();
		}, [=](std::exception_ptr exptr) {
			// On error
			_isMatching = false;
			if (_hasSubscription)
			{
				_matchmakingSubscription.unsubscribe();
				_hasSubscription = false;
			}
			try
			{
				if (exptr)
				{
					std::rethrow_exception(exptr);
				}
				else
				{
					throw std::runtime_error("Unknown error during mathmaking.");
				}
			}
			catch (const std::exception& ex)
			{
				tce.set_exception(ex);
			}
		});
		_hasSubscription = true;

		return pplx::create_task(tce);
	}

	void MatchmakingService::resolve(bool acceptMatch)
	{
		_scene->send("match.ready.resolve", [=](obytestream* stream) {
			*stream << acceptMatch;
		}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED);
	}

	void MatchmakingService::cancel()
	{
		if (_isMatching)
		{
			_isMatching = false;
			if (_hasSubscription)
			{
				_matchmakingSubscription.unsubscribe();
				_hasSubscription = false;
			}
			else
			{
				_scene->send("match.cancel", Writer(), PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::RELIABLE_ORDERED);
			}
		}
	}
};
