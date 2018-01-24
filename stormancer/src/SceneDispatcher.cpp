#include "stdafx.h"
#include "SceneDispatcher.h"
#include "IActionDispatcher.h"
#include "Scene.h"
#include "MessageIDTypes.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher)
		: _scenes((uint32)0xff - (uint32)MessageIDTypes::ID_SCENES + 1)
		, _eventDispatcher(evDispatcher)
	{
		handler = new processorFunction([=](uint8 sceneHandle, Packet_ptr packet) {
			return handler_impl(sceneHandle, packet);
		});
	}

	SceneDispatcher::~SceneDispatcher()
	{
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		config.addCatchAllProcessor(handler);
	}

	void SceneDispatcher::addScene(Scene_ptr scene)
	{
		if (scene)
		{
			uint8 index = scene->handle() - (uint8)MessageIDTypes::ID_SCENES;
			_scenes[index] = scene;
		}
	}

	void SceneDispatcher::removeScene(uint8 sceneHandle)
	{
		size_t index = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		if (index < _scenes.size())
		{
			_scenes[index].reset();
		}
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, Packet_ptr packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		unsigned int sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;

		if (sceneIndex < _scenes.size())
		{
			auto scene_weak = _scenes[sceneIndex];
			auto scene = scene_weak.lock();
			if (scene)
			{
				packet->metadata["scene"] = scene->id();
				_eventDispatcher->post([=]() {
					if (auto scene = scene_weak.lock())
					{
						scene->handleMessage(packet);
					}
				});
			}
			return true;
		}
		return false;
	}
};
