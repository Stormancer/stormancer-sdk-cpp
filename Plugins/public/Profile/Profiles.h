#pragma once

#include "Profile/ProfileModels.h"

namespace Stormancer
{
	class Profiles
	{
	public:
		virtual ~Profiles() {}
		virtual pplx::task<std::unordered_map<std::string, Profile>> getProfiles(const std::list<std::string>& userIds, const std::unordered_map<std::string, std::string>& displayOptions = { { "character", "details"} }) = 0;
		virtual pplx::task<void> updateUserHandle(const std::string& userIds) = 0;
		virtual pplx::task<std::unordered_map<std::string, Profile>> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take, const std::unordered_map<std::string, std::string>& displayOptions = { { "character", "details"} }) = 0;
	};
}
