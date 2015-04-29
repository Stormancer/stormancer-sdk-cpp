#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher()
	{
	}

	SceneDispatcher::~SceneDispatcher()
	{
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig* config)
	{
		config->addCatchAllProcessor(new processorFunction([this](byte sceneHandle, Packet<>* packet) {
			return this->processor(sceneHandle, packet);
		}));
	}

	void SceneDispatcher::addScene(Scene* scene)
	{
		uint8 index = scene->handle() - (byte)MessageIDTypes::ID_SCENES;
		_scenes[index] = scene;
	}

	void SceneDispatcher::removeScene(byte sceneHandle)
	{
		size_t index = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
		if (index < _scenes.size())
		{
			_scenes.erase(_scenes.begin() + index);
		}
	}

	bool SceneDispatcher::processor(byte sceneHandle, Packet<>* packet)
	{
		if (sceneHandle < (byte)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		size_t sceneIndex = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
		if (sceneIndex >= _scenes.size())
		{
			return false;
		}
		else
		{
			auto scene = _scenes[sceneIndex];
			packet->setMetadata(L"scene", scene);
			scene->handleMessage(packet);
			return true;
		}
	}
};
