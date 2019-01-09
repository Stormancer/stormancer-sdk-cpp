#include "GameFinder/GameFinderManager.h"
#include "Party/PartyManagement.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IClient.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RPC/Service.h"
#include "Party/PartyService.h"
#include "stormancer/exceptions.h"

namespace Stormancer
{
	PartyManagementService::PartyManagementService(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_logger = scene->dependencyResolver()->resolve<ILogger>();
	}

	pplx::task<std::string> PartyManagementService::createParty(const PartyRequestDto& partyRequestDto)
	{
		auto rpc = _scene.lock()->dependencyResolver()->resolve<RpcService>();
		return rpc->rpc<std::string, PartyRequestDto>("partymanagement.createsession", partyRequestDto);
	}

	PartyManagement::PartyManagement(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder) :ClientAPI(auth)
	{
		_auth = auth;
		_logger = logger;
		_gameFinder = gameFinder;
	}

	void PartyManagement::initialize()
	{
		auto wThat = this->weak_from_this();
		_auth.lock()->SetOperationHandler("party.invite", [wThat](OperationCtx& ctx) {
			Serializer serializer;
			auto senderId = ctx.originId;
			auto sceneId = serializer.deserializeOne<std::string>(ctx.request->inputStream());
			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("client can't accept invitations");
			}
			auto invitation = that->invitations.ReceivePartyInvitation(senderId, sceneId);
			pplx::task_completion_event<void> tce;
			auto subscription = invitation->onAnswer.subscribe([tce, ctx](bool /*answer*/)
			{
				ctx.request->sendValue([](obytestream*) {});
				tce.set();
			});
			ctx.request->cancellationToken().register_callback([wThat, senderId]()
			{
				auto that = wThat.lock();
				if (that)
				{
					that->invitations.RemovePartyInvitation(senderId);
				}
			});
			//capture subscription to keep it alive as long as the async operation is not complete
			return pplx::create_task(tce, ctx.request->cancellationToken()).then([subscription]() {});

		});

	}

	pplx::task<Stormancer::Party_ptr> PartyManagement::createParty(const PartyRequestDto& partySettings)
	{
		if (_party)
		{
			return pplx::task_from_exception<Stormancer::Party_ptr>(std::runtime_error("party.alreadyInParty"));
		}
		auto auth = _auth.lock();
		if (!auth)
		{
			return pplx::task_from_exception<Stormancer::Party_ptr>(std::runtime_error("destroyed"));
		}

		auto wThat = this->weak_from_this();
		return getPartyManagementService().then([partySettings](std::shared_ptr<Stormancer::PartyManagementService> partyManagement)
		{
			return partyManagement->createParty(partySettings);
		}).then([wThat, partySettings](pplx::task<std::string> task)
		{
			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("destroyed");
			}
			auto sceneToken = task.get();

			return that->joinPartySceneByConnectionToken(sceneToken);

		}).then([](Stormancer::Party_ptr party)
		{
			return party;
		});
	}
	pplx::task<Stormancer::Party_ptr> PartyManagement::joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName)
	{
		if (_party)
		{
			return pplx::task_from_exception<Stormancer::Party_ptr>(std::runtime_error("party.alreadyInParty"));
		}

		auto wPartyManagement = this->weak_from_this();
		auto t = leaveParty().then([wPartyManagement, uniqueOnlinePartyName]() {
			auto partyManagment = wPartyManagement.lock();
			if (partyManagment)
			{
				return partyManagment->getPartySceneByOnlinePartyName(uniqueOnlinePartyName);
			}
			else
			{
				return pplx::task_from_exception<Party_ptr>(std::runtime_error("An error occured when client try to retrieve party scene"));
			}
		}).then([wPartyManagement](pplx::task<Party_ptr> t2)
		{
			try
			{
				auto p = t2.get();
				if (auto that = wPartyManagement.lock())
				{
					that->onJoinedParty();
					that->onUpdatedPartyMembers(p->members());
					that->onUpdatedPartySettings(p->settings());
				}
				return p;
			}
			catch (std::exception& ex)
			{
				if (auto that = wPartyManagement.lock())
				{
					if (auto logger = that->_logger.lock())
					{
						logger->log(LogLevel::Error, "PartyManagement", "Failed to get the party scene.", ex.what());
					}
					that->_party = nullptr;
				}
				throw;
			}
		});
		this->_party = std::make_shared<pplx::task<Party_ptr>>(t);

		return t;

	}

	pplx::task<Stormancer::Party_ptr> PartyManagement::joinPartySceneByConnectionToken(const std::string& token)
	{
		if (_party)
		{
			return pplx::task_from_exception<Stormancer::Party_ptr>(std::runtime_error("party.alreadyInParty"));
		}

		auto wPartyManagement = this->weak_from_this();
		auto t = leaveParty().then([wPartyManagement, token]() {
			auto partyManagment = wPartyManagement.lock();
			if (partyManagment)
			{
				return partyManagment->getPartySceneByToken(token);
			}
			else
			{
				return pplx::task_from_exception<Party_ptr>(std::runtime_error("An error occured when client try to retrieve party scene"));
			}
		}).then([wPartyManagement](pplx::task<Party_ptr> t2)
		{
			try
			{
				auto p = t2.get();
				if (auto that = wPartyManagement.lock())
				{
					that->onJoinedParty();
					that->onUpdatedPartyMembers(p->members());
					that->onUpdatedPartySettings(p->settings());
				}
				return p;
			}
			catch (std::exception& ex)
			{
				if (auto that = wPartyManagement.lock())
				{
					if (auto logger = that->_logger.lock())
					{
						logger->log(LogLevel::Error, "PartyManagement", "Failed to get the party scene.", ex.what());
					}
					that->_party = nullptr;
				}
				throw;
			}
		});
		this->_party = std::make_shared<pplx::task<Party_ptr>>(t);

		return t;

	}

	pplx::task<std::shared_ptr<PartyManagementService>> PartyManagement::getPartyManagementService()
	{
		return this->getService<PartyManagementService>("stormancer.plugins.partyManagement");
	}

	pplx::task<void> PartyManagement::leaveParty()
	{
		if (!_party)
		{
			if (auto logger = _logger.lock())
			{
				logger->log(LogLevel::Warn, "PartyManagement", "Client not connected on party", "");
			}
			return pplx::task_from_result();
		}

		auto party = *_party;
		std::weak_ptr<PartyManagement> wpartyManagement = this->weak_from_this();
		auto wGameFinder = this->_gameFinder;
		return party.then([wpartyManagement](Party_ptr party)
		{
			auto partyManagement = wpartyManagement.lock();
			if (!partyManagement)
			{
				throw std::runtime_error("destroyed");
			}

			party->getScene()->disconnect();

			// FIXME: [mstorch] Not sure if this leaves some things not cleaned up???  Fixes race condition where leaving a party
			// during a party invitation acceptance caused "party.alreadyInParty" error to be returned.
			partyManagement->_party = nullptr;
		});
	}

	pplx::task<Party_ptr> PartyManagement::getParty()
	{
		if (_party)
		{
			return *_party;
		}
		else
		{
			return pplx::task_from_exception<Party_ptr>(std::runtime_error("party.notInParty"));
		}
	}

	pplx::task<void> PartyManagement::updatePlayerStatus(Stormancer::PartyUserStatus playerStatus)
	{
		return getParty().then([playerStatus](Party_ptr party)
		{
			auto partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->updatePlayerStatus(playerStatus);
		});
	}

	pplx::task<void> PartyManagement::updatePartySettings(PartySettingsDto partySettingsDto)
	{
		if (partySettingsDto.customData == "")
		{
			partySettingsDto.customData = "{}";
		}
		return getParty().then([partySettingsDto](Party_ptr party) {
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->updatePartySettings(partySettingsDto);
		});
	}

	pplx::task<void> PartyManagement::updatePlayerData(std::string data)
	{
		return getParty().then([data](Party_ptr party) {
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			partyService->updatePlayerData(data);
		});
	}

	pplx::task<bool> PartyManagement::PromoteLeader(std::string userId)
	{
		return getParty().then([userId](Party_ptr party) {
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->PromoteLeader(userId);
		});
	}

	pplx::task<bool> PartyManagement::kickPlayer(std::string userId)
	{
		return getParty().then([userId](Party_ptr party) {
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->KickPlayer(userId);
		});
	}


	pplx::task<Party_ptr> PartyManagement::getPartySceneByOnlinePartyName(const std::string uniqueOnlinePartyName)
	{
		auto auth = _auth.lock();
		auto wPartyManagement = this->weak_from_this();

		return auth->getSceneForService("stormancer.plugins.party", uniqueOnlinePartyName).then([wPartyManagement](std::shared_ptr<Scene> scene)
		{
			auto pManagement = wPartyManagement.lock();
			if (!pManagement)
			{
				throw PointerDeletedException("partyManagement");
			}
			return pManagement->initPartyFromScene(scene);

		});
	}

	pplx::task<Party_ptr> PartyManagement::getPartySceneByToken(const std::string& token)
	{
		auto auth = _auth.lock();
		auto wPartyManagement = this->weak_from_this();

		return auth->connectToPrivateSceneByToken(token).then([wPartyManagement](std::shared_ptr<Scene> scene)
		{
			auto pManagement = wPartyManagement.lock();
			if (!pManagement)
			{
				throw PointerDeletedException("partyManagement");
			}
			return pManagement->initPartyFromScene(scene);

		});
	}

	Party_ptr PartyManagement::initPartyFromScene(std::shared_ptr<Scene> scene)
	{
		try
		{
			std::weak_ptr<PartyManagement> wPartyManagement = this->shared_from_this();
			auto partyService = scene->dependencyResolver()->resolve<PartyService>();
			this->_partySceneConnectionStateSubscription = scene->getConnectionStateChangedObservable().subscribe([wPartyManagement](ConnectionState state) {
				if (auto that = wPartyManagement.lock())
				{
					if (state == ConnectionState::Disconnected)
					{

						that->onLeftParty();
						if (that->_partySceneConnectionStateSubscription.is_subscribed())
						{
							that->_partySceneConnectionStateSubscription.unsubscribe();
						}
					}
				}
			});

			/*Party(std::shared_ptr<Scene> scene,
			Action2<void>::Subscription LeftPartySubscription,
			Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
			Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
			Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription);*/
			auto party = std::make_shared<Party>(scene,
				partyService->JoinedParty.subscribe([wPartyManagement]() {
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->onJoinedParty();
				}
			}),
				partyService->KickedFromParty.subscribe([wPartyManagement]() {
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->onKickedFromParty();
				}
			}),

				partyService->LeftParty.subscribe([wPartyManagement]()
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					if (partyManagement->_party)
					{
						partyManagement->_party->then([wPartyManagement](Party_ptr party) {

							if (auto partyManagement = wPartyManagement.lock())
							{
								auto gameFinderName = party->settings().gameFinderName;
								partyManagement->_party = nullptr;


								if (auto gf = partyManagement->_gameFinder.lock())
								{
									return gf->disconnectFromGameFinder(gameFinderName);
								}

							}
							return pplx::task_from_result();

						}).then([wPartyManagement](pplx::task<void> t)
						{
							try
							{
								if (auto partyManagement = wPartyManagement.lock())
								{
									t.get();
									partyManagement->onLeftParty();
								}
							}
							catch (...) {}
						});

					}
				}
			}),

				partyService->UpdatedPartyMembers.subscribe([wPartyManagement](std::vector<PartyUserDto> partyUsers)
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->onUpdatedPartyMembers(partyUsers);
				}
			}),

				partyService->UpdatedPartyUserData.subscribe([wPartyManagement](PartyUserData partyUserUpdatedData)
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->onUpdatedUserData(partyUserUpdatedData);
				}
			}),
				partyService->UpdatedPartySettings.subscribe([wPartyManagement](Stormancer::PartySettings settings)
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->onUpdatedPartySettings(settings);
				}
			}));

			return party;
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(std::string("Party scene not found : ") + ex.what());
		}
	}
	pplx::task<void> PartyManagement::sendInvitation(const std::string& recipient)
	{
		auto wAuth = _auth;
		auto wThat = this->weak_from_this();
		return getParty().then([wAuth, wThat, recipient](Party_ptr party) {

			auto auth = wAuth.lock();
			auto that = wThat.lock();
			if (!auth || !that)
			{
				return pplx::task_from_exception<void>(std::runtime_error("destroyed"));
			}

			auto senderId = auth->userId();
			auto partyId = party->id();

			pplx::cancellation_token_source cts;

			that->invitations.SendPartyRequest(recipient, cts);
			return auth->sendRequestToUser<void>(recipient, "party.invite", cts.get_token(), senderId, partyId);

		}).then([wThat, recipient]() {


			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("destroyed");
			}
			that->invitations.ClosePartyRequest(recipient);



		});


	}

	///////////////////////////////
	///Party Invitations
	//////////////////////////////

	std::shared_ptr<Stormancer::PartyInvitation> PartyInvitations::ReceivePartyInvitation(std::string senderId, std::string sceneId)
	{
		auto invitation = std::make_shared<PartyInvitation>(senderId, sceneId);

		if (partyInvitations.find(senderId) == partyInvitations.end())
		{
			partyInvitations[senderId] = invitation;

		}
		else
		{
			throw std::runtime_error(&("Could not receive party invitation as there is already one from " + senderId)[0]);
		}
		return invitation;
	}

	void PartyInvitations::RemovePartyInvitation(std::string senderId)
	{
		if (partyInvitations.find(senderId) != partyInvitations.end())
		{
			partyInvitations.erase(senderId);
		}
		else
		{
			throw std::runtime_error(&("Could not remove pending party invitation as there is none from " + senderId)[0]);
		}
	}

	void PartyInvitations::AnswerPartyInvitation(std::string senderId, bool accept)
	{
		if (partyInvitations.find(senderId) != partyInvitations.end())
		{
			auto invitation = partyInvitations[senderId];
			if (invitation)
			{
				invitation->onAnswer(accept);
				RemovePartyInvitation(senderId);
			}
			else
			{
				throw std::runtime_error(&("Could not answer pending party invitation from " + senderId + " as it is null")[0]);
			}
		}
		else
		{
			throw std::runtime_error(&("Could not answer pending party invitation as there is none from " + senderId)[0]);
		}
	}

	std::list<std::shared_ptr<Stormancer::PartyInvitation>> PartyInvitations::GetPartyInvitations()
	{
		std::list<std::shared_ptr<Stormancer::PartyInvitation>> list;
		for (auto inv : partyInvitations)
		{
			if (inv.second)
			{
				list.push_back(inv.second);
			}
		}
		return list;
	}

	void PartyInvitations::SendPartyRequest(std::string userId, pplx::cancellation_token_source cts)
	{
		if (partyRequests.find(userId) == partyRequests.end())
		{
			partyRequests[userId] = cts;
		}
		else
		{
			throw std::runtime_error(&("Could not send a party request as there is already one for " + userId)[0]);
		}
	}

	void PartyInvitations::ClosePartyRequest(std::string userId)
	{
		if (partyRequests.find(userId) != partyRequests.end())
		{
			partyRequests[userId].cancel();
			partyRequests.erase(userId);
		}
		else
		{
			throw std::runtime_error(&("Could not cancel a party request as there none pending for " + userId)[0]);
		}
	}

	std::list<std::string> PartyInvitations::GetPartyRequestUsersIds()
	{
		std::list<std::string> list;
		for (auto request : partyRequests)
		{
			list.push_back(request.first);
		}
		return list;
	}
}


