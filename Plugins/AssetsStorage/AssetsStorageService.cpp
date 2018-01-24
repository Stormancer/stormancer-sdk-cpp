#include "stdafx.h"
#include <stormancer.h>
#include "AssetsStorageService.h"


namespace Stormancer
{
	AssetsStorageService::AssetsStorageService(Scene_ptr scene)
	{
		_scene = scene;
		_logger = scene->dependencyResolver()->resolve<ILogger>();
	}

	pplx::task<MetafileDto> AssetsStorageService::GetFile(std::string branchName, std::string path)
	{	
		auto rpcService = _scene->dependencyResolver()->resolve<RpcService>();
		return rpcService->rpc<MetafileDto, std::string, std::string>("assetsstorage.getfile", branchName, path)
		.then([](MetafileDto mfDto) {
			return mfDto;
		});
	}
}
