#pragma once

#include "stormancer/BuildConfig.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Stormancer
{
	class Scene;
	struct TransformMetadata
	{
		TransformMetadata();
		TransformMetadata(std::weak_ptr<Scene> scene);

		const std::unordered_map<std::string, std::string>& sceneMetadata();
		std::string sceneId;
		std::string routeName;
		int sceneHandle = 0;
		int routeHandle = 0;
		bool dontEncrypt = false;

	private:
		std::weak_ptr<Scene> _scene;
	};
}
