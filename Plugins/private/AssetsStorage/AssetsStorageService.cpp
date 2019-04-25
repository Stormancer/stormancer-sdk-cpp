#include "AssetsStorage/AssetsStorageService.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"


namespace Stormancer
{
	AssetsStorageService::AssetsStorageService(std::weak_ptr<Scene> scene, ILogger_ptr logger)
		: _scene(scene)
		, _logger(logger)
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
