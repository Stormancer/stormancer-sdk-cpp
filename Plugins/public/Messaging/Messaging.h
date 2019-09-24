#pragma once

#include <string>
#include "MessagingModels.h"
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	class Messaging
	{
	public:
		virtual ~Messaging() = default;
		virtual void send(const std::string& sceneId, const std::string& userId, const std::string& data) = 0;
		virtual void send(const std::string& sceneId, const std::string& userId, const std::function<void(obytestream&)>& writer) = 0;


		virtual void broadcast(const std::string& sceneId, const std::string& data) = 0;
		virtual void broadcast(const std::string& sceneId, const std::function<void(obytestream&)>& writer) = 0;

		virtual void broadcastPersistent(const std::string& sceneId, bool userMessage, const std::string& key, const std::string& data) = 0;
		virtual void broadcastPersistent(const std::string& sceneId, bool userMessage, const std::string& key, const std::function<void(obytestream&)>& writer) = 0;

		virtual pplx::task<void> deletePersistentMessage(const std::string& sceneId, bool userMessage, const std::string & key) = 0;

		Event<Message> MessageReceived;
	};
}