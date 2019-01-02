#pragma once

#include "Profile/ProfileModels.h"

namespace Stormancer
{
	class Profiles
	{
	public:
		virtual pplx::task<std::unordered_map<std::string, Profile>> getProfiles(const std::list<std::string>& userIds) = 0;
		virtual pplx::task<void> updateUserHandle(const std::string& userIds) = 0;
		virtual pplx::task<std::unordered_map<std::string, Profile>> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take) = 0;
	};
}
