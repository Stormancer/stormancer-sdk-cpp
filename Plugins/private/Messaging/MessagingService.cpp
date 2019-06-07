#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif

#include "MessagingService.h"
#include "Messaging_Impl.h"
#include "stormancer/Scene.h"
#include "stormancer/Serializer.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	Message::Message(std::shared_ptr<Scene> scene, std::string sourceId, ibytestream& stream)
		: scene(scene),
		sourceId(sourceId),
		stream(stream)
	{
	};

	Messaging_Impl::Messaging_Impl(std::shared_ptr<ILogger> logger)
		: _logger(logger)
	{
	}

	void Messaging_Impl::registerScene(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto messagingService = scene->dependencyResolver().resolve<MessagingService>();
			if (messagingService)
			{
				_logger->log(LogLevel::Error, "messaging", std::string("scene ") + scene->id() + std::string("does not support messaging"));
				return;
			}

			std::weak_ptr<Messaging_Impl> wThat = shared_from_this();
			auto subscription = messagingService->MessageReceived.subscribe([wThat](Message message)
			{
				auto that = wThat.lock();
				that->MessageReceived(message);
			});
			
			{
				std::lock_guard<std::mutex> lg(_scenesLock);
				_scenes[scene->id()] = scene;
			}

			auto sceneId = scene->id();
			scene->getConnectionStateChangedObservable().subscribe([wThat, sceneId, subscription](Stormancer::ConnectionState state)
			{
				if (state == Stormancer::ConnectionState::Disconnecting)
				{					
					subscription->unsubscribe();

					auto that = wThat.lock();
					if (that)
					{
						std::lock_guard<std::mutex> lg(that->_scenesLock);
						auto it = that->_scenes.find(sceneId);
						if (it != that->_scenes.end())
						{
							that->_scenes.erase(it);
						}
					}
				}
			});
		}
	}

	std::shared_ptr<MessagingService> Stormancer::Messaging_Impl::getMessagingService(const std::string & sceneId)
	{
		std::shared_ptr<MessagingService> result;

		{
			std::lock_guard<std::mutex> lg(_scenesLock);
			auto it = _scenes.find(sceneId);
			if (it != _scenes.end())
			{
				auto scene = it->second.lock();
				if (scene)
				{
					result = scene->dependencyResolver().resolve<MessagingService>();
					if (!result)
					{
						throw std::runtime_error(std::string("scene ") + sceneId + std::string("does not support messaging"));
					}
				}
				else
				{
					_scenes.erase(it);
				}
			}
			else
			{
				_logger->log(LogLevel::Warn, "messaging", std::string("scene ") + sceneId + std::string(" not found for messaging"));
			}
		}

		return result;
	}

	void Stormancer::Messaging_Impl::send(const std::string & sceneId, const std::string & userId, const std::string & data)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->send(userId, data);
		}
	}

	void Stormancer::Messaging_Impl::send(const std::string & sceneId, const std::string & userId, const std::function<void(obytestream&)>& writer)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->send(userId, writer);
		}
	}

	void Stormancer::Messaging_Impl::broadcast(const std::string & sceneId, const std::string & data)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->broadcast(data);
		}
	}

	void Stormancer::Messaging_Impl::broadcast(const std::string & sceneId, const std::function<void(obytestream&)>& writer)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->broadcast(writer);
		}
	}

	void Stormancer::Messaging_Impl::broadcastPersistent(const std::string & sceneId, bool userMessage, const std::string & key, const std::string & data)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->broadcastPersistent(userMessage, key, data);
		}
	}

	void Stormancer::Messaging_Impl::broadcastPersistent(const std::string & sceneId, bool userMessage, const std::string & key, const std::function<void(obytestream&)>& writer)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			messagingService->broadcastPersistent(userMessage, key, writer);
		}
	}

	pplx::task<void> Stormancer::Messaging_Impl::deletePersistentMessage(const std::string & sceneId, bool userMessage, const std::string & key)
	{
		auto messagingService = getMessagingService(sceneId);

		if (messagingService)
		{
			return messagingService->deletePersistentMessage(userMessage, key);
		}
		else
		{
			return pplx::task_from_exception<void>(std::runtime_error(std::string("scene ") + sceneId + std::string(" not found for messaging.")));
		}
	}

	MessagingService::MessagingService(std::shared_ptr<Scene> scene)
		: _scene(scene),
		_logger(scene->dependencyResolver().resolve<ILogger>())
	{
	}

	void MessagingService::initialize()
	{
		auto scene = _scene.lock();
		std::weak_ptr<MessagingService> wThat = shared_from_this();
		if (scene)
		{
			_connectionChangedSub = scene->getConnectionStateChangedObservable().subscribe([wThat](Stormancer::ConnectionState state)
			{
				if (state == Stormancer::ConnectionState::Disconnecting)
				{
					auto that = wThat.lock();
					if (that)
					{
						if (that->_connectionChangedSub.is_subscribed())
						{
							that->_connectionChangedSub.unsubscribe();
						}
						that->_scene.reset();
					}
				}
			});

			scene->addRoute("messaging.receive", [wThat](std::shared_ptr<Packet<IScenePeer>> packet)
			{
				auto that = wThat.lock();
				if (that)
				{
					auto s = that->_scene.lock();
					if (s)
					{
						that->MessageReceived(Message{ s, packet->readObject<std::string>(), packet->stream });
					}
				}
			});
		}
	}

	void MessagingService::send(const std::string& userId, const std::string& data)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			send(userId, [data](obytestream& stream)
			{
				Serializer serializer{};
				serializer.serialize(stream, data);
			});
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	void MessagingService::send(const std::string& userId, const std::function<void(obytestream&)>& writer)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			auto rpc = scene->dependencyResolver().resolve<RpcService>();
			rpc->rpc("Messaging.Send", [writer, userId](obytestream& stream)
			{
				Serializer serializer{};
				serializer.serialize(stream, userId);
				writer(stream);
			});
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	void MessagingService::broadcast(const std::string& data)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			broadcast([data](obytestream& stream)
			{
				Serializer serializer{};
				serializer.serialize(stream, data);
			});
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	void MessagingService::broadcast(const std::function<void(obytestream&)>& writer)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			auto rpc = scene->dependencyResolver().resolve<RpcService>();
			rpc->rpc("Messaging.Broadcast", writer);
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	void MessagingService::broadcastPersistent(bool userMessage, const std::string& key, const std::string& data)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			broadcastPersistent(userMessage, key, [data](obytestream& stream)
			{
				Serializer serializer{};
				serializer.serialize(stream, data);
			});
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	void MessagingService::broadcastPersistent(bool userMessage, const std::string& key, const std::function<void(obytestream&)>& writer)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			auto rpc = scene->dependencyResolver().resolve<RpcService>();
			rpc->rpc("Messaging.BroadcastPersistent", [userMessage, key, writer](obytestream& stream)
			{
				Serializer serializer{};
				serializer.serialize(stream, userMessage, key);
				writer(stream);
			});
		}
		else
		{
			_logger->log(LogLevel::Warn, "messaging", "messaging scene has been disconnected before sending data.");
		}
	}

	pplx::task<void> Stormancer::MessagingService::deletePersistentMessage(bool userMessage, const std::string & key)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			auto rpc = scene->dependencyResolver().resolve<RpcService>();
			return rpc->rpc("Messaging.DeletePersistent", userMessage, key);
		}
		else
		{
			return pplx::task_from_exception<void>(std::runtime_error("messaging scene has been disconnected."));
		}
	}
}
