#include "stormancer/stdafx.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/SceneImpl.h"

namespace Stormancer
{
	TransformMetadata::TransformMetadata() {}

	TransformMetadata::TransformMetadata(std::weak_ptr<Scene> scene)
		: _scene(scene) {}

	const std::map<std::string, std::string> _defaultMetadata;
	const std::map<std::string, std::string>& TransformMetadata::sceneMetadata()
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
