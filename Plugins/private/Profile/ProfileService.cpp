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

	pplx::task<ProfilesResult> ProfileService::getProfiles(const std::list<std::string>& userIds)
	{
		std::unordered_map<std::string, std::string> displayOptions({
			{ "character", "details"}
		});
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>, std::list<std::string>, std::unordered_map<std::string, std::string>>("profile.getprofiles", userIds, displayOptions).then([](std::unordered_map<std::string, ProfileDto> result) {
			ProfilesResult r;
			r.profiles = result;
			return r;
		});
	}

	pplx::task<void> ProfileService::updateUserHandle(const std::string& newHandle)
	{
		return _rpcService->rpc<void, std::string>("Profile.UpdateUserHandle", newHandle);
	}

	pplx::task<ProfilesResult> ProfileService::queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take)
	{
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>, std::string, int, int>("Profile.QueryProfiles", pseudoPrefix, skip, take).then([](std::unordered_map<std::string, ProfileDto> result) {
			ProfilesResult r;
			r.profiles = result;
			return r;
		});
	}
}