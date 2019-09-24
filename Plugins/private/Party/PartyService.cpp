#include "PartyService.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Scene.h"
#include "GameFinder/GameFinder.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	////////////////////////////////////////////////////////////////////////////////////
	/// PartyService
	////////////////////////////////////////////////////////////////////////////////////
	PartyService::PartyService(std::weak_ptr<Scene> scene)
		: _scene(scene)
		, _logger(scene.lock()->dependencyResolver().resolve<ILogger>())
		, _rpcService(_scene.lock()->dependencyResolver().resolve<RpcService>())
	{
		_clientReady = false;
		_playerReady = false;


	}
	void PartyService::initialize()
	{
		std::weak_ptr<PartyService> wThat = this->shared_from_this();

		_scene.lock()->addRoute("party.updatesettings", [wThat](Packetisp_ptr data)
		{
			auto updatedSettings = data->readObject<PartySettingsDto>();
			auto that = wThat.lock();
			if (that)
			{					
				that->setNewLocalSettings(updatedSettings);				
				that->setStormancerReadyStatus(updatedSettings.gameFinderName).then([wThat](pplx::task<void> task)
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
							that->_scene.lock()->disconnect();
							that->_logger->log(LogLevel::Error, "PartyService", "An error occured while trying to update the party settings", ex.what());
						}
					}
				});
			}
		});

		_scene.lock()->addRoute("party.updateuserdata", [wThat](Packetisp_ptr data)
		{
			if (auto that = wThat.lock())
			{
				auto updatedPartyUserData = data->readObject<PartyUserData>();

				that->UpdatedPartyUserData(updatedPartyUserData);
			}
		});

		_scene.lock()->addRoute("party.updatepartymembers", [wThat](Packetisp_ptr data)
		{
			if (auto that = wThat.lock())
			{
				auto members = data->readObject<std::vector<PartyUserDto>>();
				that->_members = members;
				that->UpdatedPartyMembers(members);
			}
		});

		_scene.lock()->addRoute("party.kicked", [wThat](Packetisp_ptr data)
		{
			if (auto that = wThat.lock())
			{
				that->KickedFromParty();
			}
		});

		_scene.lock()->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state) {
			if (auto that = wThat.lock())
			{
				if (state == ConnectionState::Connected)
				{
					that->JoinedParty();
				}
				else if (state == ConnectionState::Disconnected)
				{
					that->LeftParty();
				}
			}

		});

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
		if (gameFinderName != "")
		{
			auto gameFinderManager = _scene.lock()->dependencyResolver().resolve<GameFinder>();
			auto partyService = this->shared_from_this();
			return gameFinderManager->connectToGameFinder(gameFinderName).then([partyService]()
			{
				partyService->_clientReady = true;
				//partyService->sendPlayerPartyStatus();
			});
		}
		else {
			_logger->log(LogLevel::Error, "partyService", "Player isn't ready. Game finder cannot be null", "");
			return pplx::task_from_result();
		}
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

	void PartyService::setNewLocalSettings(const PartySettingsDto partySettingsDto)
	{

		_settings.gameFinderName = partySettingsDto.gameFinderName;
		_settings.leaderId = partySettingsDto.leaderId;
		_settings.customData = partySettingsDto.customData;
		_settings.partySize = partySettingsDto.partySize;
		_settings.startOnlyIfPartyFull = partySettingsDto.startOnlyIfPartyFull;

		this->UpdatedPartySettings(_settings);

	}
}
