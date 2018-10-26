#include "stormancer/stdafx.h"
#include "stormancer/headers.h"
#include "stormancer/stormancer.h"
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

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();
		return rpcService->rpc<MetafileDto>("assetsstorage.getfile", branchName, path)
			.then([](MetafileDto mfDto)
		{
			return mfDto;
		});
	}
}
