#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher()
	{
	}

	SceneDispatcher::~SceneDispatcher()
	{
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		config.addCatchAllProcessor(handler);
	}

	void SceneDispatcher::addScene(Scene& scene)
	{
		auto index = scene.handle - (byte)MessageIDTypes::ID_SCENES;
		_scenes[index] = scene;
	}

	void SceneDispatcher::removeScene(byte sceneHandle)
	{
		auto index = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
		auto it = find(_scenes.begin(), _scenes.end(), index);
		if (it != _scenes.end())
		{
			_scenes.erase(it);
		}
	}

	bool SceneDispatcher::handler(byte sceneHandle, Packet<>& packet)
	{
		if (sceneHandle < (byte)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		auto toSearch = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
		if (find(_scenes.begin(), _scenes.end(), toSearch) == _scenes.end())
		{
			return false;
		}

		auto& scene = _scenes[toSearch];
		packet.setMetadata(L"scene", &scene);
		scene.handleMessage(packet);
		return true;
	}
};
