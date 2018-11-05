#include "GameFinder/GameFinderManager.h"
#include "Party/PartyManagement.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Client.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RPC/RpcService.h"
#include "Party/PartyService.h"

namespace Stormancer
{
	///
	/// ParytManagement	
	/// Change the signature and remove scene ptr
	PartyManagementService::PartyManagementService(Scene_ptr scene)
	{
		_scene = scene;
		_logger = scene->dependencyResolver().lock()->resolve<ILogger>();
	}

	pplx::task<std::string> PartyManagementService::CreateParty(const PartyRequestDto& partyRequestDto)
	{
		auto rpc = _scene.lock()->dependencyResolver().lock()->resolve<RpcService>();
		return rpc->rpc<std::string, PartyRequestDto>("partymanagement.createsession", partyRequestDto);
	}

	PartyManagement::PartyManagement(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder)
	{
		_auth = auth;
		_logger = logger;
		_gameFinder = gameFinder;
	}

	pplx::task<Stormancer::Party_ptr> PartyManagement::CreateParty(const PartyRequestDto& partySettings)
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

		std::weak_ptr<PartyManagement> wThat = this->shared_from_this();
		return auth->getSceneForService("stormancer.plugins.partyManagement").then([wThat, partySettings](pplx::task<Stormancer::Scene_ptr> task)
		{
			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("destroyed");
			}
			auto scene = task.get();
			auto partyManagementService = scene->dependencyResolver().lock()->resolve<Stormancer::PartyManagementService>();


			return partyManagementService->CreateParty(partySettings);
		}).then([wThat, partySettings](pplx::task<std::string> task)
		{
			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("destroyed");
			}
			auto sceneToken = task.get();
			//Todo jojo rajouter la possibilité de ce connecter avec un token de connection.

			return that->joinPartyScene(partySettings.PlatformSessionId);
		}).then([](Stormancer::Party_ptr party)
		{
			return party;
		});
	}
	pplx::task<Stormancer::Party_ptr> PartyManagement::joinPartyScene(const std::string uniqueOnlinePartyName)
	{
		if (_party)
		{
			return pplx::task_from_exception<Stormancer::Party_ptr>(std::runtime_error("party.alreadyInParty"));
		}

		std::weak_ptr<PartyManagement> wPartyManagement = this->shared_from_this();
		auto t = leaveParty().then([wPartyManagement, uniqueOnlinePartyName]() {
			auto partyManagment = wPartyManagement.lock();
			if (partyManagment)
			{
				return partyManagment->getPartyScene(uniqueOnlinePartyName);
			}
			else
			{
				return pplx::task_from_exception<Party_ptr>(std::runtime_error("An error occured when client try to retrieve party scene"));
			}
		}).then([wPartyManagement](pplx::task<Party_ptr> t2)
		{
			try
			{
				return t2.get();
			}
			catch (std::exception)
			{
				if (auto that = wPartyManagement.lock())
				{
					that->_party = nullptr;
				}
				throw;
			}
		});
		this->_party = std::make_shared<pplx::task<Party_ptr>>(t);

		return t;

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
		std::weak_ptr<PartyManagement> wpartyManagement = this->shared_from_this();
		auto wGameFinder = this->_gameFinder;
		return party.then([wpartyManagement](Party_ptr party)
		{
			auto partyManagement = wpartyManagement.lock();
			if (!partyManagement)
			{
				throw std::runtime_error("destroyed");
			}

			auto gameFinderName = party->partySettings.gameFinderName;
			party->getScene()->disconnect();
			partyManagement->_party = nullptr;
			return gameFinderName;


		}).then([wGameFinder](pplx::task<std::string> task)
		{
			auto gameFinderName = task.get();
			if (auto gf = wGameFinder.lock())
			{
				return gf->disconnectFromGameFinder(gameFinderName);
			}
			else
			{
				throw std::runtime_error("destroyed");
			}
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
			return pplx::task_from_result(Party_ptr());
		}
	}

	pplx::task<void> PartyManagement::updatePlayerStatus(Stormancer::PartyUserStatus playerStatus)
	{
		return getParty().then([playerStatus](pplx::task<Stormancer::Party_ptr> task) {
			Stormancer::Party_ptr party = task.get();
			auto partyService = party->getScene()->dependencyResolver().lock()->resolve<Stormancer::PartyService>();
			return partyService->updatePlayerStatus(playerStatus);
		});
	}

	pplx::task<void> PartyManagement::updatePartySettings(PartySettingsDto partySettingsDto)
	{
		return getParty().then([partySettingsDto](pplx::task<Party_ptr> task) {
			auto party = task.get();
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver().lock()->resolve<Stormancer::PartyService>();
			return partyService->updatePartySettings(partySettingsDto);
		});
	}

	pplx::task<void> PartyManagement::updatePlayerData(std::string data)
	{
		return getParty().then([data](pplx::task<Party_ptr> task) {
			auto party = task.get();
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver().lock()->resolve<Stormancer::PartyService>();
			partyService->updatePlayerData(data);
		});
	}

	pplx::task<bool> PartyManagement::PromoteLeader(std::string userId)
	{
		return getParty().then([userId](pplx::task<Party_ptr> task) {
			auto party = task.get();
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver().lock()->resolve<Stormancer::PartyService>();
			return partyService->PromoteLeader(userId);
		});
	}

	pplx::task<bool> PartyManagement::KickPlayer(std::string userId)
	{
		return getParty().then([userId](pplx::task<Party_ptr> task) {
			auto party = task.get();
			std::shared_ptr<Stormancer::PartyService> partyService = party->getScene()->dependencyResolver().lock()->resolve<Stormancer::PartyService>();
			return partyService->KickPlayer(userId);
		});
	}


	pplx::task<Party_ptr> PartyManagement::getPartyScene(const std::string uniqueOnlinePartyName)
	{
		auto auth = _auth.lock();
		std::weak_ptr<PartyManagement> wPartyManagement = this->shared_from_this();

		return auth->getSceneForService("stormancer.plugins.partyManagement", uniqueOnlinePartyName).then([wPartyManagement, uniqueOnlinePartyName](pplx::task<Scene_ptr> task)
		{
			Scene_ptr scene = nullptr;
			try
			{
				



				auto partyService = scene->dependencyResolver().lock()->resolve<PartyService>();
				
				/*Party(Scene_ptr scene, 
				Event<void>::Subscription LeftPartySubscription,
				Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
				Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
				Event<PartySettings>::Subscription UpdatedPartySettingsSubscription);*/
				auto party = std::make_shared<Party>(task.get(),
					partyService->LeftParty.subscribe([wPartyManagement]()
				{
					if (auto partyManagement = wPartyManagement.lock())
					{
						partyManagement->onLeftParty();
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
			catch (const std::exception&)
			{
				throw std::runtime_error("Party scene not found. sceneName=" + uniqueOnlinePartyName);
			}

		});
	}
}
