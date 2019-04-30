#pragma once

#include "stormancer/Tasks.h"
#include "Profile/ProfileModels.h"
#include "Profile/Profiles.h"
#include <memory>
#include <string>
#include <list>

namespace Stormancer
{
	class Scene;
	class RpcService;
	class ILogger;

	class ProfileService : public std::enable_shared_from_this<ProfileService>
	{
	public:
		ProfileService(std::shared_ptr<Scene> scene);
		~ProfileService();
		ProfileService(const ProfileService& other) = delete;
		ProfileService(const ProfileService&& other) = delete;
		ProfileService& operator=(const ProfileService&& other) = delete;

		pplx::task<ProfilesResult> getProfiles(const std::list<std::string>& userIds, const std::unordered_map<std::string, std::string>& displayOptions = Profiles::defaultDisplayOptions);
		pplx::task<ProfileDto> getProfile(const std::string& userId, const std::unordered_map<std::string, std::string>& displayOptions = Profiles::defaultDisplayOptions);
		pplx::task<ProfilesResult> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take, const std::unordered_map<std::string, std::string>& displayOptions = Profiles::defaultDisplayOptions);
		pplx::task<void> updateUserHandle(const std::string& userIds);

	private:
		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "Profile";
	};
}
