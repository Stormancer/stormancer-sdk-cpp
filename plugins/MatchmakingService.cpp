#include "MatchmakingService.h"

namespace Stormancer
{
	MatchmakingService::MatchmakingService(Stormancer::Scene* scene)
		: _scene(scene),
		_rpcService(scene->dependencyResolver()->resolve<Stormancer::IRpcService>())
	{
		_scene->addRoute("match.update", [this](Stormancer::Packetisp_ptr packet) {
			byte matchStateByte;
			packet->stream->read((char*)&matchStateByte, 1);
			Stormancer::int32 matchState = matchStateByte;

			_matchState = (MatchState)matchState;

			auto ms = std::to_string(matchState);

			_onMatchUpdate(_matchState);

			if (_matchState == MatchState::Success)
			{
				msgpack::unpacked result;

				std::string buffer;
				*packet->stream >> buffer;
				auto off = msgpack::unpack(result, buffer.data(), buffer.size());
				Internal::MatchmakingResponse mmres;
				result.get().convert(&mmres);

				MatchmakingResponse mmres2;

				mmres2.optionalParameters = mmres.optionalParameters;
				mmres2.team1 = mmres.team1;
				mmres2.team2 = mmres.team2;

				_onMatchFound(mmres2);
			}
		});

		_scene->addRoute("match.parameters.update", [this](Stormancer::Packetisp_ptr packet) {
			std::string buffer;
			*packet->stream >> buffer;

			msgpack::unpacked result;
			auto off = msgpack::unpack(result, buffer.data(), buffer.size());
			std::string provider;
			result.get().convert(&provider);

			msgpack::unpacked result3;
			off += msgpack::unpack(result3, buffer.data() + off, buffer.size() - off);
			result3.get().convert(&_matchmakingRequest);

			_onMatchParametersUpdate(_matchmakingRequest);
		});

		_scene->addRoute("match.ready.update", [this](Stormancer::Packetisp_ptr packet) {
			std::string buffer;
			*packet->stream >> buffer;
			msgpack::unpacked result;
			msgpack::unpack(result, buffer.data(), buffer.size());
			Internal::ReadyVerificationRequest readyUpdate;
			result.get().convert(&readyUpdate);

			ReadyVerificationRequest readyUpdate2;
			readyUpdate2.matchId = readyUpdate.matchId;
			readyUpdate2.timeout = readyUpdate.timeout;
			readyUpdate2.membersCountTotal = readyUpdate2.members.size();
			readyUpdate2.membersCountReady = 0;
			for (auto it : readyUpdate2.members)
			{
				readyUpdate2.members[it.first] = it.second;
				if (it.second == Readiness::Ready)
				{
					readyUpdate2.membersCountReady++;
				}
			}

			if (_onMatchReadyUpdate)
			{
				_onMatchReadyUpdate(readyUpdate2);
			}
		});
	}

	MatchmakingService::~MatchmakingService()
	{
		auto sub = _matchmakingSubscription.lock();
		if (sub)
		{
			sub->unsubscribe();
		}
	}

	void MatchmakingService::onMatchUpdate(std::function<void(MatchState)> callback)
	{
		_onMatchUpdate = callback;
	}

	void MatchmakingService::onMatchParametersUpdate(std::function<void(MatchmakingRequest)> callback)
	{
		_onMatchParametersUpdate = callback;
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

	pplx::task<std::shared_ptr<Stormancer::Result<>>> MatchmakingService::findMatch(std::string provider, std::map<std::string, std::string> profileIds)
	{
		pplx::task_completion_event<std::shared_ptr<Stormancer::Result<>>> tce;

		_isMatching = true;

		MatchmakingRequest mmreq;
		mmreq.profileIds = profileIds;


		auto observable = _rpcService->rpc("match.find", [provider, mmreq](Stormancer::bytestream* stream) {
			msgpack::pack(stream, provider);
			msgpack::pack(stream, mmreq);
		}, PacketPriority::MEDIUM_PRIORITY);

		auto onNext = [this, tce](Stormancer::Packetisp_ptr packet) {
			_isMatching = false;
			auto sub = _matchmakingSubscription.lock();
			if (sub)
			{
				sub->unsubscribe();
			}
			std::shared_ptr<Stormancer::Result<>> res(new Stormancer::Result<>());
			//auto res = new std::shared_ptr<Stormancer::Result<>*>();
			res->set();
			tce.set(res);
		};

		auto onError = [this, tce](const char* error) {
			_isMatching = false;
			auto sub = _matchmakingSubscription.lock();
			if (sub)
			{
				sub->unsubscribe();
			}

			std::shared_ptr<Stormancer::Result<>> res(new Stormancer::Result<>());
			//auto res = new std::shared_ptr<Stormancer::Result<>*>();
			res->setError(1, error);
			tce.set(res);
		};

		auto onComplete = []() {
		};

		_matchmakingSubscription = observable->subscribe(onNext, onError, onComplete);

		return pplx::create_task(tce);
	}

	void MatchmakingService::resolve(bool acceptMatch)
	{
		_scene->sendPacket("match.ready.resolve", [acceptMatch](Stormancer::bytestream* stream) {
			*stream << acceptMatch;
		}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
	}

	void MatchmakingService::cancel()
	{
		if (_isMatching)
		{
			_isMatching = false;
			auto sub = _matchmakingSubscription.lock();
			if (sub)
			{
				sub->unsubscribe();
			}
			else
			{
				_scene->sendPacket("match.cancel", [](Stormancer::bytestream* stream) {}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
			}
		}
	}
};
