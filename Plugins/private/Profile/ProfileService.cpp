#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "ProfileService.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	ProfileService::ProfileService(std::shared_ptr<Scene> scene)
		: _scene(scene)
		, _rpcService(scene->dependencyResolver()->resolve<RpcService>())
		, _logger(scene->dependencyResolver()->resolve<ILogger>())
	{
	}

	ProfileService::~ProfileService()
	{
	}

	pplx::task<ProfilesResult> ProfileService::getProfiles(const std::list<std::string>& userIds, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>>("profile.getprofiles", userIds, displayOptions)
			.then([](std::unordered_map<std::string, ProfileDto> result)
		{
			ProfilesResult r;
			r.profiles = result;
			return r;
		});
	}

	pplx::task<ProfileDto> ProfileService::getProfile(const std::string& userId, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return getProfiles(std::list<std::string> { userId }, displayOptions)
			.then([userId](ProfilesResult profiles)
		{
			if (profiles.profiles.size() == 1)
			{
				return profiles.profiles[userId];
			}
			else
			{
				throw std::runtime_error("No profile");
			}
		});
	}

	pplx::task<void> ProfileService::updateUserHandle(const std::string& newHandle)
	{
		return _rpcService->rpc("Profile.UpdateUserHandle", newHandle);
	}

	pplx::task<ProfilesResult> ProfileService::queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take, const std::unordered_map<std::string, std::string>& displayOptions)
	{
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>>("Profile.QueryProfiles", pseudoPrefix, skip, take, displayOptions)
			.then([](std::unordered_map<std::string, ProfileDto> result)
		{
			ProfilesResult r;
			r.profiles = result;
			return r;
		});
	}
}
