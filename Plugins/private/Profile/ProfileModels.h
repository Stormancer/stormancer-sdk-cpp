#pragma once
#include <string>
#include <unordered_map>
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	struct PlayerProfileDto
	{
		std::string Id;
		std::string PlayerId;
		std::string Pseudo;
		std::string SteamId;
		MSGPACK_DEFINE(Id, PlayerId, Pseudo, SteamId)
	};

	struct Profile
	{
		std::unordered_map<std::string, std::string> data;
		
	};

	struct ProfileDto
	{
		std::unordered_map<std::string, std::string> Data;
		MSGPACK_DEFINE(Data)
	};
	
	struct ProfilesResult
	{
		std::unordered_map<std::string, ProfileDto> profiles;
	};
}