#pragma once
#include <string>
#include <unordered_map>
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	struct Profile
	{
		std::unordered_map<std::string, std::string> data;
	};

	struct ProfileDto
	{
		std::unordered_map<std::string, std::string> data;
		MSGPACK_DEFINE(data);
	};
	
	struct ProfilesResult
	{
		std::unordered_map<std::string, ProfileDto> profiles;
	};
}
