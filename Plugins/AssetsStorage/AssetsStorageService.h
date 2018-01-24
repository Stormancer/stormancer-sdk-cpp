#pragma once
#include "headers.h"
#include <stormancer.h>

namespace Stormancer
{
	class MetafileDto
	{
	public:
		MSGPACK_DEFINE(FileName, URL, Path, MD5Hash)
		std::string FileName;
		std::string URL;
		std::string Path;
		std::string MD5Hash;
	};

	class AssetsStorageService
	{
	public:
		AssetsStorageService(Stormancer::Scene_ptr scene);
		Scene_ptr GetScene() {
			return this->_scene;
		}

		pplx::task<MetafileDto> GetFile(std::string branchName, std::string path);
	private:
		Scene_ptr _scene;
		std::shared_ptr<Stormancer::ILogger> _logger;		
	};
}