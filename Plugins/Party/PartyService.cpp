#include "stormancer/stormancer.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Scene.h"
#include "Party/PartyService.h"
#include "GameFinder/GameFinderManager.h"
#include "Party/PartyManagement.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	////////////////////////////////////////////////////////////////////////////////////
	/// PartyService
	////////////////////////////////////////////////////////////////////////////////////
	PartyService::PartyService(Scene_ptr scene)
		: _scene(scene),
		_rpcService(_scene->dependencyResolver().lock()->resolve<RpcService>()),
		_logger(scene->dependencyResolver().lock()->resolve<ILogger>())
	{
		_clientReady = false;
		_playerReady = false;

		std::weak_ptr<PartyService> wThat = this->shared_from_this();

		_scene->addRoute("party.updatesettings", [wThat](Packetisp_ptr data)
		{
			auto updatedSettings = data->readObject<PartySettingsDto>();
			auto that = wThat.lock();
			if (that)
			{

				that->setNewLocalSettings(updatedSettings).then([wThat, updatedSettings]()
				{
					auto that = wThat.lock();
					if (that)
					{
						return that->setStormancerReadyStatus(updatedSettings.gameFinderName);
					}
					else
					{
						throw std::runtime_error("destroyed");
					}

				}).then([wThat](pplx::task<void> task)
				{
					try
					{
						task.get();
					}
					catch (const std::exception& ex)
					{
						auto that = wThat.lock();
						if (that)
						{
							that->_scene->disconnect();
							that->_logger->log(LogLevel::Error, "PartyService", "An error occured while trying to update the party settings", ex.what());
						}
					}
				});
			}
		});

		_scene->addRoute("party.updateuserdata", [this](Packetisp_ptr data)
		{
			auto updatedPartyUserData = data->readObject<PartyUserData>();

			UpdatedPartyUserData(updatedPartyUserData);

		});

		_scene->addRoute("party.updatepartymembers", [this](Packetisp_ptr data)
		{
			auto members = data->readObject<std::vector<PartyUserDto>>();
			_members = members;
			this->UpdatedPartyMembers(members);

		});
	}

	std::vector<PartyUserDto> PartyService::members()
	{
		return _members;
	}

	pplx::task<void> PartyService::updatePartySettings(const PartySettingsDto partySettings)
	{
		return _rpcService->rpc<void>("party.updatepartysettings", partySettings);
	}

	void PartyService::updatePlayerStatus(const PartyUserStatus newStatus)
	{
		_playerReady = false;
		if (newStatus == PartyUserStatus::Ready)
		{
			_playerReady = true;
		}
		sendPlayerPartyStatus();
	}

	pplx::task<void> PartyService::updatePlayerData(std::string data)
	{

		return _rpcService->rpc<void>("party.updatepartyuserdata", data);
	}


	void PartyService::onDisconnected()
	{
		LeftParty();
	}

	pplx::task<bool> PartyService::PromoteLeader(const std::string playerId)
	{
		return _rpcService->rpc<bool>("party.promoteleader", playerId);
	}

	pplx::task<bool> PartyService::KickPlayer(const std::string playerId)
	{
		return _rpcService->rpc<bool>("party.kickplayer", playerId);
	}

	pplx::task<void> PartyService::setStormancerReadyStatus(const std::string gameFinderName)
	{
		_clientReady = false;
		auto gameFinderManager = _scene->dependencyResolver().lock()->resolve<GameFinder>();
		auto partyService = this->shared_from_this();
		return gameFinderManager->connectToGameFinder(gameFinderName).then([partyService]()
		{
			partyService->_clientReady = true;
			partyService->sendPlayerPartyStatus();
		});
	}

	void PartyService::sendPlayerPartyStatus()
	{
		//Check if player is ready and if the stormancer is ready.
		auto partyService = this->shared_from_this();
		if (_playerReady && _clientReady)
		{
			auto sendStatus = PartyUserStatus::Ready;
			_rpcService->rpc<void>("party.updategamefinderplayerstatus", sendStatus).then([partyService](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& ex)
				{
					partyService->_logger->log(LogLevel::Error, "PartyService", "An error occured when party try to update player status", ex.what());
				}
			});
		}
		else
		{
			auto sendStatus = PartyUserStatus::NotReady;
			_rpcService->rpc<void>("party.updategamefinderplayerstatus", sendStatus).then([partyService](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& ex)
				{
					partyService->_logger->log(LogLevel::Error, "PartyService", "An error occured when party try to update player status", ex.what());
				}
			});
		}
	}

	pplx::task<void> PartyService::setNewLocalSettings(const PartySettingsDto partySettingsDto)
	{
		std::weak_ptr<PartyService> wThat = this->shared_from_this();
		auto partyManagement = _scene->dependencyResolver().lock()->resolve<PartyManagement>();
		return partyManagement->getParty().then([wThat, partySettingsDto](Party_ptr party)
		{
			party->partySettings.gameFinderName = partySettingsDto.gameFinderName;
			party->partySettings.leaderId = partySettingsDto.leaderId;
			party->partySettings.customData = partySettingsDto.customData;
			party->partySettings.partySize = partySettingsDto.partySize;
			auto that = wThat.lock();
			if (that)
			{
				that->UpdatedPartySettings(party->partySettings);
			}
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	/// Party
	////////////////////////////////////////////////////////////////////////////////////
	Party::Party(Scene_ptr scene,
		Action2<void>::Subscription LeftPartySubscription,
		Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
		Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
		Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription) :
		_partyScene(scene),
		LeftPartySubscription(LeftPartySubscription),
		UpdatedPartyMembersSubscription(UpdatedPartyMembersSubscription),
		UpdatedPartyUserDataSubscription(UpdatedPartyUserDataSubscription),
		UpdatedPartySettingsSubscription(UpdatedPartySettingsSubscription)
	{

	}

	Stormancer::Scene_ptr Party::getScene()
	{
		return _partyScene;
	}

	bool Party::isLeader()
	{
		return partySettings.leaderId == _partyScene->dependencyResolver().lock()->resolve<AuthenticationService>()->userId();
	}
}
