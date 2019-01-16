#pragma once
#include <memory>
#include "stormancer/Event.h"
#include "Party/PartyModels.h"
#include "GameFinder/GameFinderModels.h"

namespace Stormancer
{
	//Forward declares
	class Scene;
	class ILogger;
	class RpcService;

	class PartyService : public std::enable_shared_from_this<PartyService>
	{
	public:
		PartyService(std::weak_ptr<Scene> scene);

		///
		/// Sent to server the new party status
		///
		pplx::task<void> updatePartySettings(const PartySettingsDto newPartySettings);

		///
		/// Update the player. If the client and player are ready. The ready status is 
		/// send to server.
		///
		void updatePlayerStatus(const PartyUserStatus newStatus);

		/// 
		/// Update party user data all data are replecated between all connected party scene
		/// 
		pplx::task<void> updatePlayerData(std::string data);


		///
		/// Promote player to leader of the party
		/// \param playerId party userid will be promote
		pplx::task<bool> PromoteLeader(const std::string playerId);

		///
		/// Remove player from party this method can be call only by party leader.
		/// \param playerToKick is the user player id to be kicked
		pplx::task<bool> KickPlayer(const std::string playerId);

		///
		/// Callback member
		///
		Event<GameFinderStatus> PartyGameFinderStateUpdated;
		Event<GameFinderResponse> onPartyMatchFound;

		Event<void> LeftParty;
		Event<void> JoinedParty;
		Event<void> KickedFromParty;
		Event<std::vector<PartyUserDto>> UpdatedPartyMembers;		
		Event<PartyUserData> UpdatedPartyUserData;
		Event<PartySettings> UpdatedPartySettings;

		std::vector<PartyUserDto>& members() {
			return _members;
		}
		
		PartySettings& settings()
		{
			return _settings;
		}

		void initialize();
	private:

		PartySettings _settings;
		///
		/// Connect the client to gameFinder
		///
		pplx::task<void> setStormancerReadyStatus(const std::string gameFinder);

		///
		/// Send the new player status on party scene.
		///
		void sendPlayerPartyStatus();

		void setNewLocalSettings(const PartySettingsDto partySettings);

		std::shared_ptr<ILogger> _logger;
		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;
		
		bool _playerReady;
		bool _clientReady;
		std::vector<PartyUserDto> _members;
	};

	
}