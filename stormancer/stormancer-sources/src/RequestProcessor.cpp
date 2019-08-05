#include "stormancer/stdafx.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/Serializer.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	void RequestProcessor::Initialize(std::shared_ptr<RequestProcessor> processor, std::vector<std::shared_ptr<IRequestModule>> modules)
	{
		std::weak_ptr<RequestProcessor> wProcessor = processor;
		RequestModuleBuilder builder([wProcessor](byte msgId, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)> handler)
		{
			auto processor = LockOrThrow(wProcessor);
			processor->addSystemRequestHandler(msgId, handler);
		});

		for (size_t i = 0; i < modules.size(); i++)
		{
			auto module = modules[i];
			module->registerModule(builder);
		}
	}

	RequestProcessor::RequestProcessor(std::shared_ptr<ILogger> logger)
		: _logger(logger)
	{
	}

	void RequestProcessor::addSystemRequestHandler(byte msgId, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)> handler)
	{
		if (_isRegistered)
		{
			throw std::runtime_error("Can only add handler before 'RegisterProcessor' is called.");
		}
		_handlers.emplace(msgId, handler);
	}

	void RequestProcessor::registerProcessor(PacketProcessorConfig& config)
	{
		_isRegistered = true;

		auto logger = _logger;
		auto serializer = _serializer;

		auto wProcessor = STORM_WEAK_FROM_THIS();

		config.addProcessor((byte)MessageIDTypes::ID_SYSTEM_REQUEST, [wProcessor, logger, serializer](Packet_ptr packet)
		{
			auto processor = wProcessor.lock();
			if (!processor)
			{
				return false;
			}

			byte sysRequestId;
			packet->stream >> sysRequestId;
			std::shared_ptr<RequestContext> context = std::make_shared<RequestContext>(packet);
			auto handlerIt = processor->_handlers.find(sysRequestId);

			if (handlerIt == processor->_handlers.end())
			{
				context->error([serializer](obytestream& stream)
				{
					std::string message = "No system request handler found.";
					serializer.serialize(stream, message);
				});
				return true;
			}

			invokeWrapping(handlerIt->second, context)
				.then([logger, serializer, context](pplx::task<void> t)
			{
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					if (!context->isComplete())
					{
						context->error([serializer, ex](obytestream& stream)
						{
							std::string message = std::string() + "An error occured on the server. " + ex.what();
							serializer.serialize(stream, message);
						});
					}
					else
					{
						logger->log(LogLevel::Trace, "RequestProcessor", "An error occured", ex.what());
					}
					return;
				}

				if (!context->isComplete())
				{
					context->complete();
				}
			});

			return true;
		});

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, [wProcessor, logger](Packet_ptr packet)
		{
			auto processor = wProcessor.lock();
			if (!processor)
			{
				return false;
			}

			uint16 id;
			packet->stream >> id;

			auto request = processor->freeRequestSlot(id);

			auto idStr = std::to_string(id);

			if (request)
			{
				packet->metadata["request"] = idStr;
				request->setLastRefresh(std::chrono::system_clock::now());
				if (!request->trySet(packet))
				{
					logger->log(LogLevel::Warn, "RequestProcessor/next", "Can't set the system request", idStr);
				}
			}
			else
			{
				logger->log(LogLevel::Warn, "RequestProcessor/next", "Unknown request id", idStr);
			}

			return true;
		});

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, [wProcessor, logger](Packet_ptr p)
		{
			auto processor = wProcessor.lock();
			if (!processor)
			{
				return false;
			}

			uint16 id;
			p->stream >> id;

			byte hasValuesData;
			p->stream >> hasValuesData;
			bool hasValues = (hasValuesData != 0);

			if (!hasValues)
			{
				SystemRequest_ptr request = processor->freeRequestSlot(id);

				auto idStr = std::to_string(id);

				if (request)
				{
					p->metadata["request"] = idStr;
					if (!request->trySet(Packet_ptr()))
					{
						logger->log(LogLevel::Warn, "RequestProcessor/complete", "Can't set the system request", idStr);
					}
				}
				else
				{
					logger->log(LogLevel::Warn, "RequestProcessor/complete", "Unknown request id", idStr);
				}
			}

			return true;
		});

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, [wProcessor, serializer, logger](Packet_ptr p)
		{
			auto processor = wProcessor.lock();
			if (!processor)
			{
				return false;
			}

			uint16 id;
			p->stream >> id;

			SystemRequest_ptr request = processor->freeRequestSlot(id);

			auto idStr = std::to_string(id);

			if (request)
			{
				p->metadata["request"] = idStr;
				std::string msg = serializer.deserializeOne<std::string>(p->stream);
				std::string message = msg + "(msgId:" + std::to_string(request->operation()) + ")";
				if (!request->trySetException(std::runtime_error(message.c_str())))
				{
					logger->log(LogLevel::Warn, "RequestProcessor/error", "Can't set the exception in the system request", idStr);
				}
			}
			else
			{
				logger->log(LogLevel::Warn, "RequestProcessor/error", "Unknown request id", idStr);
			}

			return true;
		});
	}

	pplx::task<Packet_ptr> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, const StreamWriter& streamWriter, PacketPriority priority, pplx::cancellation_token ct)
	{
		if (!peer)
		{
			return pplx::task_from_exception<Packet_ptr>(std::invalid_argument("peer should not be nullptr"));
		}

		auto request = reserveRequestSlot(msgId, ct);
		auto requestId = request->getId();
		auto wRequestProcessor = STORM_WEAK_FROM_THIS();
		_logger->log(LogLevel::Trace, "RequestProcessor", "Sending system request " + std::to_string(msgId) + " to " + std::to_string(peer->id()));
		if (ct.is_cancelable())
		{
			ct.register_callback([requestId, wRequestProcessor]()
			{
				if (auto requestProcessor = wRequestProcessor.lock())
				{
					requestProcessor->freeRequestSlot(requestId);
					// don't try to cancel the tce, it will cancel the associated tasks on delete
				}
			});
		}

		try
		{
			TransformMetadata metadata;
			if (msgId == (byte)SystemRequestIDTypes::ID_SET_METADATA)
			{
				metadata.dontEncrypt = true; // SET metadata contains the encryption key
			}
			peer->send([msgId, request, &streamWriter](obytestream& stream)
			{
				stream << (byte)MessageIDTypes::ID_SYSTEM_REQUEST;
				stream << msgId;
				stream << request->getId();
				if (streamWriter)
				{
					streamWriter(stream);
				}
			}, systemRequestChannelUid, priority, PacketReliability::RELIABLE, metadata);
		}
		catch (const std::exception& ex)
		{
			request->trySetException(ex);
		}

		return request->getTask(ct);
	}

	SystemRequest_ptr RequestProcessor::reserveRequestSlot(byte msgId, pplx::cancellation_token ct)
	{
		SystemRequest_ptr request;

		{ // this scope ensures the static id is not used outside the locked mutex.
			std::lock_guard<std::mutex> lock(_mutexPendingRequests);

			static uint16 id = 0;
			// i is used to know if we tested all uint16 available values, whatever the current value of id.
			for (uint32 i = 0; i <= 0xffff; i++)
			{
				id++;

				if (!mapContains(_pendingRequests, id))
				{
					request = std::make_shared<SystemRequest>(msgId, id, ct);
					_pendingRequests[id] = request;
					break;
				}
				else
				{
					std::string unexpectedMsgId;
					if (_pendingRequests[id])
					{
						unexpectedMsgId = "msgId: " + std::to_string(_pendingRequests[id]->operation());
					}
					else
					{
						unexpectedMsgId = "request is null";
					}
					_logger->log(LogLevel::Warn, "RequestProcessor", "Unexpected occupied request slot: " + std::to_string(id), unexpectedMsgId);
				}
			}
		}

		if (request)
		{
			request->setLastRefresh(std::chrono::system_clock::now());
			return request;
		}

		throw std::overflow_error("Unable to create a new request: Too many pending requests.");
	}

	SystemRequest_ptr RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		SystemRequest_ptr request;

		std::lock_guard<std::mutex> lock(_mutexPendingRequests);

		auto it = _pendingRequests.find(requestId);
		if (it != _pendingRequests.end())
		{
			request = it->second;
			_pendingRequests.erase(it);
		}

		return request;
	}
}
