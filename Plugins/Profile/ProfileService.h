#pragma once
#include <memory>
#include <unordered_map>
#include "stormancer/msgpack_define.h"
#include "pplx/pplxtasks.h"


namespace Stormancer
{
	class Scene;
	class RpcService;
	class ILogger;

	struct ProfileDto
	{
		std::unordered_map<std::string, std::string> Data;
		MSGPACK_DEFINE(Data)
	};
	struct ProfilesResult
	{
		std::unordered_map<std::string, ProfileDto> profiles;
	};
	class ProfileService : public std::enable_shared_from_this<ProfileService>
	{
	public:
		ProfileService(std::shared_ptr<Scene> scene);
		~ProfileService();
		ProfileService(const ProfileService& other) = delete;
		ProfileService(const ProfileService&& other) = delete;
		ProfileService& operator=(const ProfileService&& other) = delete;

		pplx::task<ProfilesResult> getProfiles(const std::list<std::string>& userIds);
		pplx::task<void> updateUserHandle(const std::string& userIds);
		pplx::task<ProfilesResult> queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take);

	private:
		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "Profile";
	};
}