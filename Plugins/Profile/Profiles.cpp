#include "stormancer/headers.h"
#include "Profiles.h"
#include "Profile/ProfileService.h"


namespace Stormancer
{
	Profiles::Profiles(std::weak_ptr<AuthenticationService> auth) :ClientAPI(auth) {}

	pplx::task<std::unordered_map<std::string, Profile>> Profiles::getProfiles(const std::list<std::string>& userIds)
	{
		return getProfileService()
			.then([userIds](std::shared_ptr<ProfileService> gr) {return gr->getProfiles(userIds); })
			.then([](ProfilesResult profiles) {
			std::unordered_map<std::string, Profile> result;
			for (auto& dto : profiles.profiles)
			{
				Profile p;
				p.data = dto.second.Data;
				result.emplace(dto.first, p);
			}
			return result;

		});
	}

	pplx::task<void> Profiles::updateUserHandle(const std::string& userIds)
	{
		return getProfileService()
			.then([userIds](std::shared_ptr<ProfileService> gr) {return gr->updateUserHandle(userIds); });
	}

	pplx::task<std::unordered_map<std::string, Profile>> Profiles::queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take)
	{
		return getProfileService()
			.then([pseudoPrefix, skip, take](std::shared_ptr<ProfileService> gr) {return gr->queryProfiles(pseudoPrefix, skip, take); })
			.then([](ProfilesResult profiles) {
			std::unordered_map<std::string, Profile> result;
			for (auto& dto : profiles.profiles)
			{
				Profile p;
				p.data = dto.second.Data;
				result.emplace(dto.first, p);
			}
			return result;

		});
	}


	pplx::task<std::shared_ptr<ProfileService>> Profiles::getProfileService()
	{
		return this->template getService<ProfileService>("stormancer.profiles");
	}
}
