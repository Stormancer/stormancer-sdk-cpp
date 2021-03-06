#include "stormancer/stdafx.h"
#include "stormancer/SceneDispatcher.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher, std::shared_ptr<ILogger> logger)
		: _eventDispatcher(evDispatcher),
		logger(logger)
	{
		handler = [this](uint8 sceneHandle, Packet_ptr packet)
		{
			return handler_impl(sceneHandle, packet);
		};
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		config.addCatchAllProcessor(handler);
	}

	uint8 SceneDispatcher::getSceneIndex(uint8 sceneHandle)
	{
		return sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
	}

	void SceneDispatcher::resize(std::vector<std::weak_ptr<Scene_Impl>>& handles, uint8 index)
	{
		if ((uint8)handles.capacity() < index + 1)
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
			handles.resize(newSize);
		}
	}

	std::shared_ptr<std::vector<std::weak_ptr<Scene_Impl>>> SceneDispatcher::getHandles(const IConnection& connection)
	{
		return connection.dependencyResolver().resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
	}

	void SceneDispatcher::addScene(std::shared_ptr<IConnection> connection, std::shared_ptr<Scene_Impl> scene)
	{
		if (scene && connection)
		{
			auto handles = getHandles(*connection);
			if (handles)
			{
				auto index = getSceneIndex(scene->host()->handle());
				resize(*handles, index);
				(*handles)[index] = scene;
			}
		}
	}

	void SceneDispatcher::removeScene(std::shared_ptr<IConnection> connection,uint8 sceneHandle)
	{
		if (connection)
		{
			auto index = getSceneIndex(sceneHandle);
			auto handles = getHandles(*connection);
			if (handles)
			{
				if (index < handles->size())
				{
					(*handles)[index].reset();
				}
			}
		}
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, Packet_ptr packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		logger->log(LogLevel::Trace, "SceneDispatcher", "Received message for sceneHandle " + std::to_string(sceneHandle), packet->connection->ipAddress());
		unsigned int sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;

		auto connection = packet->connection;
		auto handles = getHandles(*connection);
		if (sceneIndex < handles->size())
		{
			auto scene_weak = (*handles)[sceneIndex];
			auto scene = scene_weak.lock();
			if (scene)
			{
				packet->metadata["scene"] = scene->id();
				_eventDispatcher->post([=]()
				{
					if (auto scene = scene_weak.lock())
					{
						logger->log(LogLevel::Trace, "SceneDispatcher", "handling message for scene " + scene->id(), packet->connection->ipAddress());
						scene->handleMessage(packet);
					}
				});
			}
			return true;
		}
		return false;
	}

	std::shared_ptr<Scene_Impl> Stormancer::SceneDispatcher::getScene(std::shared_ptr<IConnection> connection, uint8 sceneHandle)
	{
		auto handles = getHandles(*connection);
		auto index = getSceneIndex(sceneHandle);
		if (handles && index < handles->size())
		{
			return (*handles)[index].lock();
		}
		else
		{
			return nullptr;
		}
	}
}
