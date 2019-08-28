#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Profiles_Impl.h"
#include "ProfileService.h"
#include "Users/Users.hpp"

namespace Stormancer
{
	const std::unordered_map<std::string, std::string> Profiles::defaultDisplayOptions = { { "character", "details" } };

	Profiles_Impl::Profiles_Impl(std::weak_ptr<Users::UsersApi> users) : ClientAPI(users) {}

	pplx::task<std::unordered_map<std::string, Profile>> Profiles_Impl::getProfiles(const std::list<std::string>& userIds, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return getProfileService()
			.then([userIds, displayOptions](std::shared_ptr<ProfileService> gr) {return gr->getProfiles(userIds, displayOptions); })
			.then([](ProfilesResult profiles) {
			std::unordered_map<std::string, Profile> result;
			for (auto& dto : profiles.profiles)
			{
				Profile p;
				p.data = dto.second.data;
				result.emplace(dto.first, p);
			}
			return result;
		});
	}

	pplx::task<Profile> Profiles_Impl::getProfile(const std::string& userId, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return getProfileService()
			.then([userId, displayOptions](std::shared_ptr<ProfileService> gr)
		{
			return gr->getProfile(userId, displayOptions);
		})
			.then([](ProfileDto profile)
		{
			Profile p;
			p.data = profile.data;
			return p;
		});
	}

	pplx::task<void> Profiles_Impl::updateUserHandle(const std::string& userIds)
	{
		return getProfileService()
			.then([userIds](std::shared_ptr<ProfileService> gr) {return gr->updateUserHandle(userIds); });
	}

	pplx::task<std::unordered_map<std::string, Profile>> Profiles_Impl::queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return getProfileService()
			.then([pseudoPrefix, skip, take, displayOptions](std::shared_ptr<ProfileService> gr) {return gr->queryProfiles(pseudoPrefix, skip, take, displayOptions); })
			.then([](ProfilesResult profiles) {
			std::unordered_map<std::string, Profile> result;
			for (auto& dto : profiles.profiles)
			{
				Profile p;
				p.data = dto.second.data;
				result.emplace(dto.first, p);
			}
			return result;
		});
	}

	pplx::task<std::shared_ptr<ProfileService>> Profiles_Impl::getProfileService()
	{
		return this->template getService<ProfileService>("stormancer.profiles");
	}
}
