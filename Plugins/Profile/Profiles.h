#pragma once
#include "Core/ClientAPI.h"


namespace Stormancer
{
	struct Profile
	{
		std::unordered_map<std::string, std::string> data;
		
	};

	class ProfileService;

	class Profiles : public ClientAPI<Profiles>
	{
	public:
		Profiles(std::weak_ptr<AuthenticationService> auth);
		pplx::task<std::unordered_map<std::string, Profile>> getProfiles(const std::list<std::string>& userIds);
		pplx::task<void> updateUserHandle(const std::string& userIds);
		pplx::task<std::unordered_map<std::string, Profile>> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take);
	private:
		pplx::task<std::shared_ptr<ProfileService>> getProfileService();
	};
}
