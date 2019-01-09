#pragma once
#include "stormancer/headers.h"
#include "AssetsStorage/AssetsStorageDto.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/scene.h"

namespace Stormancer
{
	
	class AssetsStorageService
	{
	public:

		AssetsStorageService(std::weak_ptr<Scene> scene, ILogger_ptr logger);
		std::weak_ptr<Scene> GetScene();
		pplx::task<MetafileDto> GetFile(std::string branchName, std::string path);

	private:

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<Stormancer::ILogger> _logger;		
	};
}
