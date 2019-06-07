#include "stormancer/stdafx.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/SceneImpl.h"

namespace Stormancer
{
	const std::unordered_map<std::string, std::string> _defaultMetadata;

	TransformMetadata::TransformMetadata()
	{
	}

	TransformMetadata::TransformMetadata(std::weak_ptr<Scene> scene)
		: _scene(scene)
	{
	}

	const std::unordered_map<std::string, std::string>& TransformMetadata::sceneMetadata()
	{
		auto scene = _scene.lock();
		if (scene)
		{
			return scene->getSceneMetadata();
		}
		else
		{
			return _defaultMetadata;
		}
	}
}
