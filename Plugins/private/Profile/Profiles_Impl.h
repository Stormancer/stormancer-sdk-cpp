#pragma once
#include "Core/ClientAPI.h"
#include "Profile/Profiles.h"

namespace Stormancer
{
	class ProfileService;
	class AuthenticationService;

	class Profiles_Impl : public ClientAPI<Profiles_Impl>, public Profiles
	{
	public:
		Profiles_Impl(std::weak_ptr<AuthenticationService> auth);
		pplx::task<std::unordered_map<std::string, Profile>> getProfiles(const std::list<std::string>& userIds) override;
		pplx::task<void> updateUserHandle(const std::string& userIds) override;
		pplx::task<std::unordered_map<std::string, Profile>> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take) override;
	private:
		pplx::task<std::shared_ptr<ProfileService>> getProfileService();
	};
}
