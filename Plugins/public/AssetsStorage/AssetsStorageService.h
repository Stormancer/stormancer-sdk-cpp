#pragma once

#include "AssetsStorage/AssetsStorageDto.h"
#include "stormancer/Tasks.h"
#include "stormancer/Logger/ILogger.h"
#include <memory>
#include <string>

namespace Stormancer
{
	class Scene;
	
	class AssetsStorageService
	{
	public:

		AssetsStorageService(std::weak_ptr<Scene> scene, ILogger_ptr logger);
		std::weak_ptr<Scene> GetScene();
		pplx::task<MetafileDto> GetFile(std::string branchName, std::string path);

	private:

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<ILogger> _logger;
	};
}
