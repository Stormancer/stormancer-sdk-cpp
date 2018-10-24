#pragma once
//#include "stormancer/stormancer.h"
#include <memory>
#include "GameFinder/GameFinderService.h"

//Forward declares
class Scene;
class ILogger;
class RpcService;

namespace Stormancer
{
	enum class PartyUserStatus
	{
		unknown = -1,
		NotReady = 0,
		Ready = 1
	};

	struct PartyUserDto
	{		
		std::string userId;	
		bool isLeader;
		PartyUserStatus partyUserStatus;
		std::string userData;		
		MSGPACK_DEFINE(userId, isLeader, partyUserStatus, userData)
	};

	struct PartyUserData
	{
		std::string userId;
		std::string userData;
		MSGPACK_DEFINE(userId, userData)
	};


	struct PartySettingsDto
	{
		/// <summary>
		/// The name of game finder scene
		/// </summary>
		std::string gameFinderName;

		/// <summary>
		/// The leader user id
		/// </summary>
		std::string leaderId;

		uint16 partySize;

		std::string customData;

		MSGPACK_DEFINE(gameFinderName, leaderId,partySize, customData)
	};
	

	struct PartySettings
	{
		///
		/// The name of game finder scene
		///
		std::string gameFinderName;

		/// 
		/// The leader user id
		///
		std::string leaderId;

		///
		/// Max number of slot in party
		///
		uint16 partySize;

		std::string customData;
	};

	class Party;

	using Party_ptr = std::shared_ptr<Party>;

	class PartyService : public std::enable_shared_from_this<PartyService>
	{
	public:
		PartyService(Scene_ptr scene);

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
		/// Call when client is disconnected from scene
		///
		void onDisconnected();

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
		Action2<GameFinderStatus> PartyGameFinderStateUpdated;
		Action2<GameFinderResponse> onPartyMatchFound;

		Action2<void> LeftParty;

		Action2<std::vector<PartyUserDto>> UpdatedPartyMembers;
		Action2<PartyUserData> UpdatedPartyUserData;
		Action2<PartySettings> UpdatedPartySettings;

		std::vector<PartyUserDto> members();
	private:

		///
		/// Connect the client to gameFinder
		///
		pplx::task<void> setStormancerReadyStatus(const std::string gameFinder);

		///
		/// Send the new player status on party scene.
		///
		void sendPlayerPartyStatus();

		pplx::task<void> setNewLocalSettings(const PartySettingsDto partySettings);

		std::shared_ptr<ILogger> _logger;
		Scene_ptr _scene;
		std::shared_ptr<RpcService> _rpcService;
		
		bool _playerReady;
		bool _clientReady;
		std::vector<PartyUserDto> _members;
	};

	class Party
	{
	public:
		Party(Scene_ptr scene, 
			Action2<void>::Subscription LeftPartySubscription,
			Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
			Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
			Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription);

		PartySettings partySettings;

		std::vector<PartyUserDto> members()
		{
			return _partyScene->dependencyResolver().lock()->resolve<PartyService>()->members();
		}
		bool isLeader();
		Scene_ptr getScene();

	private:
		Action2<void>::Subscription LeftPartySubscription;

		Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription;
		Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription;
		Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription;

		
		Scene_ptr _partyScene;
	};
}
MSGPACK_ADD_ENUM(Stormancer::PartyUserStatus)