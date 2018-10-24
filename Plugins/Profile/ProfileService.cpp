#include "stormancer/Logger/ILogger.h"
#include "stormancer/RPC/RpcService.h"
#include "ProfileService.h"

namespace Stormancer
{
	ProfileService::ProfileService(std::shared_ptr<Scene> scene)
		: _scene(scene)
		, _rpcService(scene->dependencyResolver().lock()->resolve<RpcService>())
		, _logger(scene->dependencyResolver().lock()->resolve<ILogger>())
	{
	}

	ProfileService::~ProfileService()
	{
	}

	pplx::task<std::unordered_map<std::string, ProfileDto>> ProfileService::getProfiles(const std::list<std::string>& userIds)
	{
		std::unordered_map<std::string, std::string> displayOptions({
			{ "character", "details"}
		});
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>, std::list<std::string>, std::unordered_map<std::string, std::string>>("profile.getprofiles", userIds, displayOptions);
	}

	pplx::task<void> ProfileService::updateUserHandle(const std::string& newHandle)
	{
		return _rpcService->rpc<void, std::string>("Profile.UpdateUserHandle", newHandle);
	}

	pplx::task<std::unordered_map<std::string, ProfileDto>> ProfileService::queryProfiles(const std::string& pseudoPrefix, const int& skip, const int& take)
	{
		return _rpcService->rpc<std::unordered_map<std::string, ProfileDto>, std::string, int, int>("Profile.QueryProfiles", pseudoPrefix, skip, take);
	}
}