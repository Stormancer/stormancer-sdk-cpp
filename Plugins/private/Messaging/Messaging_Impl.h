#pragma once
#include "Messaging/Messaging.h"
#include <memory>
#include <unordered_map>
#include <string>


namespace Stormancer
{
	class Scene;
	class MessagingService;
	class ILogger;

	class Messaging_Impl: public Messaging, public std::enable_shared_from_this<Messaging_Impl>
	{
	public:
		Messaging_Impl(std::shared_ptr<ILogger> logger);
		void registerScene(std::shared_ptr<Scene> scene);

		void send(const std::string& sceneId, const std::string& userId, const std::string& data) override;
		void send(const std::string& sceneId, const std::string& userId, const std::function<void(obytestream&)>& writer)  override;


		void broadcast(const std::string& sceneId, const std::string& data)  override;
		void broadcast(const std::string& sceneId, const std::function<void(obytestream&)>& writer)  override;

		void broadcastPersistent(const std::string& sceneId, bool userMessage, const std::string& key, const std::string& data)  override;
		void broadcastPersistent(const std::string& sceneId, bool userMessage, const std::string& key, const std::function<void(obytestream&)>& writer)  override;

		pplx::task<void> deletePersistentMessage(const std::string& sceneId, bool userMessage, const std::string & key) override;

	private:		
		std::mutex _scenesLock;
		std::unordered_map<std::string, std::weak_ptr<Scene>> _scenes;
		std::shared_ptr<ILogger> _logger;

		std::shared_ptr<MessagingService> getMessagingService(const std::string& sceneId);
	};
}