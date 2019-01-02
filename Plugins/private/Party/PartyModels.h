#pragma once

#include "stormancer/Event.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include <string>

namespace Stormancer
{	
	struct PartyRequestDto
	{
		std::string PlatformSessionId;
		uint64 PartySize;
		std::string GameFinderName;
		std::string CustomData;
		bool StartOnlyIfPartyFull;
		MSGPACK_DEFINE(PlatformSessionId, PartySize, GameFinderName, CustomData, StartOnlyIfPartyFull);
	};

	class PartyInvitation
	{
	public:

		std::string UserId;
		std::string SceneId;
		PartyInvitation(std::string userId, std::string sceneId)
		{
			UserId = userId;
			SceneId = sceneId;
		}

		Event<bool> onAnswer;
	};
	
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
}
MSGPACK_ADD_ENUM(Stormancer::PartyUserStatus)