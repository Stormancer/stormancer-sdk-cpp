#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Tasks.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	namespace Chat
	{
		class ChatPlugin;

		enum class UserStatus
		{
			Connected,
			Disconnected,
		};

		namespace details
		{

			struct ChatUserInfoDto
			{
				std::string userId;
				std::string sessionId;
				UserStatus status;
				std::unordered_map<std::string, std::string> metadata;
				MSGPACK_DEFINE(userId, sessionId, status, metadata)
			};
			struct ChatMessageDto
			{
				std::string message;
				std::unordered_map<std::string, std::string> metadata;
				uint64 timestamp;
				ChatUserInfoDto userInfos;

				MSGPACK_DEFINE(message, metadata, timestamp, userInfos)

			};
			
			

			class ChatService :public std::enable_shared_from_this<ChatService>
			{
				friend class Stormancer::Chat::ChatPlugin;
			public:
				ChatService(std::shared_ptr<Stormancer::RpcService> rpc) : _rpcService(rpc)
				{
				}

				pplx::task<void> send(std::string& message, std::unordered_map<std::string, std::string>& metadata)
				{
					auto rpc = _rpcService.lock();
					if (!rpc)
					{
						throw Stormancer::PointerDeletedException("Scene destroyed");
					}
					return rpc->rpc<void>("Chat.Message", message, metadata);
				}

				pplx::task<std::vector<ChatMessageDto>> getHistory(uint64 startDate, uint64 endDate)
				{
					auto rpc = _rpcService.lock();
					if (!rpc)
					{
						throw Stormancer::PointerDeletedException("Scene destroyed");
					}
					return rpc->rpc<std::vector<ChatMessageDto>>("Chat.LoadHistory", startDate, endDate);
				}
				//Event fired whenever the service client receives a server message on the route Chat.MessageReceived.
				Stormancer::Event<ChatMessageDto> msgReceived;
				Stormancer::Event<ChatUserInfoDto> statusChanged;

			private:

				std::weak_ptr<Stormancer::RpcService> _rpcService;

				//Initializes the service
				void initialize(std::shared_ptr<Stormancer::Scene> scene)
				{
					//Capture a weak pointer of this in the route handler to make sure that:
					//* We don't prevent this from being destroyed (capturing a shared pointer)
					//* If destroyed, we don't try to use it in the handler (capturing a this reference directly)
					std::weak_ptr<ChatService> wService = this->shared_from_this();
					scene->addRoute("Chat.MessageReceived", [wService](Stormancer::Packetisp_ptr packet) {
						//We expect the message to contain a string
						auto message = packet->readObject<ChatMessageDto>();
						auto service = wService.lock();
						//If service is valid, forward the event.
						if (service)
						{
							service->msgReceived(message);
						}

						});
					scene->addRoute("Chat.StatusChanged", [wService](Stormancer::Packetisp_ptr packet) {
						//We expect the message to contain a string
						auto message = packet->readObject<std::vector<ChatUserInfoDto>>();
						auto service = wService.lock();
						//If service is valid, forward the event.
						if (service)
						{
							for (auto change : message)
							{
								service->statusChanged(change);
							}
						}

						});
				}

			};

			struct ChannelContainer
			{
				std::shared_ptr<ChatService> service;
				Stormancer::Event<ChatMessageDto>::Subscription msgReceivedSubscription;
				Stormancer::Event<ChatUserInfoDto>::Subscription statusChangedSubscription;
			};
		}


		struct ChatMsg
		{
			std::string sceneId;
			details::ChatMessageDto message;
		};
		struct StatusUpdate
		{
			std::string sceneId;
			details::ChatUserInfoDto user;
		};

		class Chat : public std::enable_shared_from_this<Chat>
		{
			friend class ChatPlugin;
		public:

			Chat() {}
			pplx::task<void> send(std::string id, std::string message, std::unordered_map<std::string, std::string> metadata)
			{
				auto it = _channels.find(id);
				if (it == _channels.end())
				{
					return pplx::task_from_exception<void>(std::runtime_error(("notConnected?scene=" + id).c_str()));
				}

				return it->second.service->send(message, metadata);
			}
			pplx::task<std::vector<ChatMsg>> getHistory(std::string id,uint64 startTimestamp, uint64 endTimestamp)
			{
				auto it = _channels.find(id);
				if (it == _channels.end())
				{
					return pplx::task_from_exception<std::vector<ChatMsg>>(std::runtime_error(("notConnected?scene=" + id).c_str()));
				}
				return it->second.service->getHistory(startTimestamp, endTimestamp).then([id](std::vector<details::ChatMessageDto> messages)
				{
					std::vector<ChatMsg> result;
					for (auto message : messages)
					{
						ChatMsg msg;
						msg.sceneId = id;
						msg.message = message;
						result.push_back(msg);
					}
					return result;
					
				});
			}
			std::vector<std::string> channels()
			{
				std::vector<std::string> keys;
				for (auto container : _channels)
				{
					keys.push_back(container.first);
				}
				return keys;
			}
			Stormancer::Event<ChatMsg> msgReceived;
			Stormancer::Event<StatusUpdate> statusUpdated;

			Stormancer::Event<std::string> channelConnected;
			Stormancer::Event<std::string> channelDisconnected;


		private:
			void onConnecting(std::string id, std::shared_ptr<details::ChatService> service)
			{
				details::ChannelContainer container;
				container.service = service;

				std::weak_ptr<Chat> wThis = this->shared_from_this();
				//Always capture weak references, and NEVER 'this'. As the callback is going to be executed asynchronously,
				//who knows what may have happened to the object behind the this pointer since it was captured?
				container.msgReceivedSubscription = service->msgReceived.subscribe([wThis, id](details::ChatMessageDto dto) {
					auto that = wThis.lock();
					//If this is valid, forward the event.
					if (that)
					{
						ChatMsg msg;
						msg.sceneId = id;
						msg.message = dto;
					
						that->msgReceived(msg);
					}
				});

				container.statusChangedSubscription = service->statusChanged.subscribe([wThis, id](details::ChatUserInfoDto dto) {

					auto that = wThis.lock();
					//If this is valid, forward the event.
					if (that)
					{
						StatusUpdate msg;
						msg.sceneId = id;
						msg.user = dto;

						that->statusUpdated(msg);
					}
					
				});

				_channels.emplace(id, container);
				this->channelConnected(id);
			}
			void onDisconnecting(std::string id)
			{
				this->channelDisconnected(id);
				_channels.erase(id);

			}

			std::unordered_map<std::string, details::ChannelContainer> _channels;

		};

		class ChatPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{

				auto name = scene->getHostMetadata("stormancer.chat");

				if (!name.empty())
				{
					builder.registerDependency<details::ChatService, Stormancer::RpcService>().singleInstance();
				}

			}
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<Chat>().singleInstance();
			}
			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.chat");

				if (!name.empty())
				{
					auto service = scene->dependencyResolver().resolve<details::ChatService>();
					service->initialize(scene);
				}
			}

			void sceneConnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.chat");

				if (!name.empty())
				{
					auto hello = scene->dependencyResolver().resolve<Chat>();
					auto service = scene->dependencyResolver().resolve<details::ChatService>();
					hello->onConnecting(scene->id(),service);
				}
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.chat");

				if (!name.empty())
				{
					auto hello = scene->dependencyResolver().resolve<Chat>();
				
					hello->onDisconnecting(scene->id());
				}
			}
		};

	}

}
MSGPACK_ADD_ENUM(Stormancer::Chat::UserStatus)