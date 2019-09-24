#pragma once
#include <memory>
#include "rxcpp/rx.hpp" 
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "Messaging/MessagingModels.h"


namespace Stormancer
{
	class Scene;
	class ILogger;
	class obytestream;

	class MessagingService : public std::enable_shared_from_this<MessagingService>
	{
	public:

		MessagingService(std::shared_ptr<Scene> scene);

		void send(const std::string& userId, const std::string& data);
		void send(const std::string& userId, const std::function<void(obytestream&)>& writer);


		void broadcast(const std::string& data);
		void broadcast(const std::function<void(obytestream&)>& writer);

		void broadcastPersistent(bool userMessage, const std::string& key, const std::string& data);
		void broadcastPersistent(bool userMessage, const std::string& key, const std::function<void(obytestream&)>& writer);

		pplx::task<void> deletePersistentMessage(bool userMessage, const std::string& key);

		Event<Message> MessageReceived;
		
	private:
		void initialize();

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<ILogger> _logger;
		rxcpp::subscription _connectionChangedSub;
	};
}
