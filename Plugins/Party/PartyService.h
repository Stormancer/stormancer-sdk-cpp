#pragma once
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

		/// <summary>
		/// Party size
		/// </summary>
		uint16 partySize;

		std::string customData;

		bool startOnlyIfPartyFull;

		MSGPACK_DEFINE(gameFinderName, leaderId,partySize, customData, startOnlyIfPartyFull)
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

		bool startOnlyIfPartyFull;
	};

	class Party;

	using Party_ptr = std::shared_ptr<Party>;

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
		Action2<GameFinderStatus> PartyGameFinderStateUpdated;
		Action2<GameFinderResponse> onPartyMatchFound;

		Action2<void> LeftParty;
		Action2<void> JoinedParty;
		Action2<void> KickedFromParty;
		Action2<std::vector<PartyUserDto>> UpdatedPartyMembers;		
		Action2<PartyUserData> UpdatedPartyUserData;
		Action2<PartySettings> UpdatedPartySettings;

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

	class Party
	{
	public:
		Party(std::shared_ptr<Scene> scene,
			Action2<void>::Subscription JoinedPartySubscription,
			Action2<void>::Subscription LeftPartySubscription,
			Action2<void>::Subscription KickedFromPartySubscription,
			Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
			Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
			Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription);

		bool is_settings_valid() const
		{
			return (_partyScene != nullptr && _partyScene->dependencyResolver()->resolve<PartyService>());
		}

		PartySettings& settings()
		{
			return  _partyScene->dependencyResolver()->resolve<PartyService>()->settings();
		}

		std::vector<PartyUserDto>& members()
		{
			return _partyScene->dependencyResolver()->resolve<PartyService>()->members();
		}

		bool isLeader();
		std::shared_ptr<Scene> getScene();
		std::string id()
		{
			return _partyScene->id();
		}
	private:
		Action2<void>::Subscription LeftPartySubscription;
		Action2<void>::Subscription JoinedPartySubscription;
		Action2<void>::Subscription KickedFromPartySubscription;
		Action2<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription;
		Action2<PartyUserData>::Subscription UpdatedPartyUserDataSubscription;
		Action2<PartySettings>::Subscription UpdatedPartySettingsSubscription;

		
		std::shared_ptr<Scene> _partyScene;
	};
}
MSGPACK_ADD_ENUM(Stormancer::PartyUserStatus)