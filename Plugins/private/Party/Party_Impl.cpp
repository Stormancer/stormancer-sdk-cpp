#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Party_Impl.h"
#include "PartyService.h"
#include "PartyContainer.h"
#include "GameFinder/GameFinder_Impl.h"
#include "stormancer/Exceptions.h"

namespace Stormancer
{
	Party_Impl::Party_Impl(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder) :ClientAPI(auth)
	{
		_logger = logger;
		_gameFinder = gameFinder;
	}

	void Party_Impl::initialize()
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

	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::createParty(const PartyRequestDto& partySettings)
	{
		if (_party)
		{
			return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("party.alreadyInParty"));
		}
		auto auth = _auth.lock();
		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("destroyed"));
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

		}).then([](std::shared_ptr<PartyContainer> party)
		{
			return party;
		});
	}
	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName)
	{
		if (_party)
		{
			return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("party.alreadyInParty"));
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
				return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("An error occured when client try to retrieve party scene"));
			}
		}).then([wPartyManagement](pplx::task<std::shared_ptr<PartyContainer>> t2)
		{
			try
			{
				auto p = t2.get();
				if (auto that = wPartyManagement.lock())
				{
					that->_onJoinedParty();
					//that->_onUpdatedPartyMembers(p->members());
					//that->_onUpdatedPartySettings(p->settings());
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
		this->_party = std::make_shared<pplx::task<std::shared_ptr<PartyContainer>>>(t);

		return t;

	}

	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::joinPartySceneByConnectionToken(const std::string& token)
	{
		if (_party)
		{
			return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("party.alreadyInParty"));
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
				return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("An error occured when client try to retrieve party scene"));
			}
		}).then([wPartyManagement](pplx::task<std::shared_ptr<PartyContainer>> t2)
		{
			try
			{
				auto p = t2.get();
				if (auto that = wPartyManagement.lock())
				{
					that->_onJoinedParty();
					that->_onUpdatedPartyMembers(p->members());
					that->_onUpdatedPartySettings(p->settings());
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

		this->_party = std::make_shared<pplx::task<std::shared_ptr<PartyContainer>>>(t);

		return t;

	}

	pplx::task<std::shared_ptr<PartyManagementService>> Party_Impl::getPartyManagementService()
	{
		return this->getService<PartyManagementService>("stormancer.plugins.partyManagement");
	}

	pplx::task<void> Party_Impl::leaveParty()
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
		std::weak_ptr<Party_Impl> wpartyManagement = this->weak_from_this();
		auto wGameFinder = this->_gameFinder;
		return party.then([wpartyManagement](std::shared_ptr<PartyContainer> party)
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

	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::getParty()
	{
		if (_party)
		{
			return *_party;
		}
		else
		{
			return pplx::task_from_exception<std::shared_ptr<PartyContainer>>(std::runtime_error("party.notInParty"));
		}
	}

	pplx::task<void> Party_Impl::updatePlayerStatus(PartyUserStatus playerStatus)
	{
		return getParty().then([playerStatus](std::shared_ptr<PartyContainer> party)
		{
			auto partyService = party->getScene()->dependencyResolver()->resolve<PartyService>();
			return partyService->updatePlayerStatus(playerStatus);
		});
	}

	pplx::task<void> Party_Impl::updatePartySettings(PartySettingsDto partySettingsDto)
	{
		if (partySettingsDto.customData == "")
		{
			partySettingsDto.customData = "{}";
		}
		return getParty().then([partySettingsDto](std::shared_ptr<PartyContainer> party) {
			std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver()->resolve<PartyService>();
			return partyService->updatePartySettings(partySettingsDto);
		});
	}

	pplx::task<void> Party_Impl::updatePlayerData(std::string data)
	{
		return getParty().then([data](std::shared_ptr<PartyContainer> party) {
			std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver()->resolve<PartyService>();
			partyService->updatePlayerData(data);
		});
	}

	pplx::task<bool> Party_Impl::PromoteLeader(std::string userId)
	{
		return getParty().then([userId](std::shared_ptr<PartyContainer> party) {
			std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->PromoteLeader(userId);
		});
	}

	pplx::task<bool> Party_Impl::kickPlayer(std::string userId)
	{
		return getParty().then([userId](std::shared_ptr<PartyContainer> party) {
			std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver()->resolve<Stormancer::PartyService>();
			return partyService->KickPlayer(userId);
		});
	}


	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::getPartySceneByOnlinePartyName(const std::string uniqueOnlinePartyName)
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

	pplx::task<std::shared_ptr<PartyContainer>> Party_Impl::getPartySceneByToken(const std::string& token)
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

	std::shared_ptr<PartyContainer> Party_Impl::initPartyFromScene(std::shared_ptr<Scene> scene)
	{
		try
		{
			std::weak_ptr<Party_Impl> wPartyManagement = this->shared_from_this();
			auto partyService = scene->dependencyResolver()->resolve<PartyService>();
			this->_partySceneConnectionStateSubscription = scene->getConnectionStateChangedObservable().subscribe([wPartyManagement](ConnectionState state) {
				if (auto that = wPartyManagement.lock())
				{
					if (state == ConnectionState::Disconnected)
					{

						that->_onLeftParty();
						if (that->_partySceneConnectionStateSubscription.is_subscribed())
						{
							that->_partySceneConnectionStateSubscription.unsubscribe();
						}
					}
				}
			});

			auto party = std::make_shared<PartyContainer>(scene,
				partyService->JoinedParty.subscribe([wPartyManagement]() {
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->_onJoinedParty();
				}
			}),
				partyService->KickedFromParty.subscribe([wPartyManagement]() {
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->_onKickedFromParty();
				}
			}),

				partyService->LeftParty.subscribe([wPartyManagement]()
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					if (partyManagement->_party)
					{
						partyManagement->_party->then([wPartyManagement](std::shared_ptr<PartyContainer> party) {

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
									partyManagement->_onLeftParty();
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
					partyManagement->_onUpdatedPartyMembers(partyUsers);
				}
			}),

				partyService->UpdatedPartyUserData.subscribe([wPartyManagement](PartyUserData partyUserUpdatedData)
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->_onUpdatedUserData(partyUserUpdatedData);
				}
			}),
				partyService->UpdatedPartySettings.subscribe([wPartyManagement](Stormancer::PartySettings settings)
			{
				if (auto partyManagement = wPartyManagement.lock())
				{
					partyManagement->_onUpdatedPartySettings(settings);
				}
			}));

			return party;
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(std::string("Party scene not found : ") + ex.what());
		}
	}
	pplx::task<void> Party_Impl::sendInvitation(const std::string& recipient)
	{
		auto wAuth = _auth;
		auto wThat = this->weak_from_this();
		return getParty().then([wAuth, wThat, recipient](std::shared_ptr<PartyContainer> party) {

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
	Event<PartySettings>::Subscription Party_Impl::subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback)
	{
		return _onUpdatedPartySettings.subscribe(callback);
	}
	Event<std::vector<PartyUserDto>>::Subscription Party_Impl::subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback)
	{
		return _onUpdatedPartyMembers.subscribe(callback);
	}
	Event<PartyUserData>::Subscription Party_Impl::subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback)
	{
		return _onUpdatedUserData.subscribe(callback);
	}
	Event<void>::Subscription Party_Impl::subscribeOnJoinedParty(std::function<void()> callback)
	{
		return _onJoinedParty.subscribe(callback);
	}
	Event<void>::Subscription Party_Impl::subscribeOnKickedFromParty(std::function<void()> callback)
	{
		return _onKickedFromParty.subscribe(callback);
	}
	Event<void>::Subscription Party_Impl::subscribeOnLeftParty(std::function<void()> callback)
	{
		return _onLeftParty.subscribe(callback);
	}
}
