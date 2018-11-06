#include "stormancer/stdafx.h"
#include "stormancer/SceneDispatcher.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher)
		: _eventDispatcher(evDispatcher)
		/*, _scenes((uint32)0xff - (uint32)MessageIDTypes::ID_SCENES + 1)*/
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

	void SceneDispatcher::addScene(std::shared_ptr<IConnection> connection, Scene_ptr scene)
	{
		if (scene && connection)
		{
			auto handles = connection->dependencyResolver()->resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
			uint8 index = scene->handle() - (uint8)MessageIDTypes::ID_SCENES;
			auto& v = *handles;
			if (v.capacity() < index + 1)
			{
				auto newSize = (index + 1) * 2;
				if (newSize < 8)
				{
					newSize = 8;
				}
				if (newSize > 150)
				{
					newSize = 150;
				}
				v.resize(newSize);
				
			}
			v[index] = scene;
		}
	}

	void SceneDispatcher::removeScene(std::shared_ptr<IConnection> connection,uint8 sceneHandle)
	{
		size_t index = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		auto handles = connection->dependencyResolver()->resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
		if (index < handles->size())
		{
			(*handles)[index].reset();
		}
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, Packet_ptr packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		unsigned int sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;

		auto connection = packet->connection;
		auto handles = connection->dependencyResolver()->resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
		if (sceneIndex < handles->size())
		{
			auto scene_weak = (*handles)[sceneIndex];
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
