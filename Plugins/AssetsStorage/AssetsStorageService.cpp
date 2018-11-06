#include "stormancer/headers.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/scene.h"
#include "stormancer/RPC/service.h"
#include "AssetsStorageService.h"

namespace Stormancer
{
	AssetsStorageService::AssetsStorageService(std::weak_ptr<Scene> scene, ILogger_ptr logger)
		: _logger(logger)
		, _scene(scene)
	{
	}

	std::weak_ptr<Scene> AssetsStorageService::GetScene()
	{
		return _scene;
	}

	pplx::task<MetafileDto> AssetsStorageService::GetFile(std::string branchName, std::string path)
	{	
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<MetafileDto>(std::runtime_error("scene deleted"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();
		return rpcService->rpc<MetafileDto>("assetsstorage.getfile", branchName, path)
			.then([](MetafileDto mfDto)
		{
			return mfDto;
		});
	}
}
