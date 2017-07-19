#include <stdafx.h>
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
		_scene->addRoute("match.update", [this](Packetisp_ptr packet) {
			byte matchStateByte;
			packet->stream->read((char*)&matchStateByte, 1);
			int32 matchState = matchStateByte;

			_matchState = (MatchState)matchState;

			auto ms = std::to_string(matchState);

			_onMatchUpdate(_matchState);

			if (_matchState == MatchState::Success)
			{
				msgpack::unpacked result;

				std::string buffer;
				*packet->stream >> buffer;

				//std::string bufferLisible = StringifyBytesArray(buffer, true);
				//UE_LOG(LogTemp, Error, TEXT("buffer value -----------------> :  %s"), *(FString(bufferLisible.c_str())));

				msgpack::unpack(result, buffer.data(), buffer.size());
				MatchmakingResponse mmres;
				result.get().convert(&mmres);

				_onMatchFound(mmres);
			}
		});

		_scene->addRoute("match.parameters.update", [this](Packetisp_ptr packet) {
			std::string buffer;
			*packet->stream >> buffer;

			msgpack::unpacked result;
			msgpack::unpack(result, buffer.data(), buffer.size());
			std::string provider;
			result.get().convert(&provider);

			//_onMatchParametersUpdate();
		});

		_scene->addRoute("match.ready.update", [this](Packetisp_ptr packet) {
			std::string buffer;
			*packet->stream >> buffer;
			msgpack::unpacked result;
			msgpack::unpack(result, buffer.data(), buffer.size());
			Internal::ReadyVerificationRequest readyUpdateTmp;
			result.get().convert(&readyUpdateTmp);

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

		rxcpp::observable<Packetisp_ptr> observable = _rpcService->rpc_observable("match.find", [provider](bytestream* stream) {
			msgpack::pack(stream, provider);
		}, PacketPriority::MEDIUM_PRIORITY);

		auto onNext = [this, tce](Packetisp_ptr packet) {
			_isMatching = false;
			if (_hasSubscription)
			{
				_matchmakingSubscription.unsubscribe();
				_hasSubscription = false;
			}
			tce.set();
		};

		auto onError = [this, tce](std::exception_ptr eptr) {
			_isMatching = false;
			if (_hasSubscription)
			{
				_matchmakingSubscription.unsubscribe();
				_hasSubscription = false;
			}
			try
			{
				if (eptr)
				{
					std::rethrow_exception(eptr);
				}
				else
				{
					throw std::runtime_error("Unknown error during mathmaking.");
				}
			}
			catch(std::exception& ex)
			{
				tce.set_exception(ex);
			}
		};

		_matchmakingSubscription = observable.subscribe(onNext, onError);
		_hasSubscription = true;

		return pplx::create_task(tce);
	}

	void MatchmakingService::resolve(bool acceptMatch)
	{
		_scene->sendPacket("match.ready.resolve", [acceptMatch](bytestream* stream) {
			*stream << acceptMatch;
		}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
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
				_scene->sendPacket("match.cancel", [](bytestream*) {}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
			}
		}
	}
};
