#include "stormancer/stdafx.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/Serializer.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/SafeCapture.h"
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

		auto wProcessor = STRM_WEAK_FROM_THIS();

		config.addProcessor((byte)MessageIDTypes::ID_SYSTEM_REQUEST, [wProcessor, logger, serializer](Packet_ptr p)
		{
			auto processor = LockOrThrow(wProcessor);

			byte sysRequestId;
			p->stream >> sysRequestId;
			std::shared_ptr<RequestContext> context = std::make_shared<RequestContext>(p);
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

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, [wProcessor, logger](Packet_ptr p)
		{
			auto processor = LockOrThrow(wProcessor);

			uint16 id;
			p->stream >> id;

			SystemRequest_ptr request = processor->freeRequestSlot(id);

			if (request)
			{
				p->metadata["request"] = std::to_string(request->id);
				time(&request->lastRefresh);
				if (!request->complete)
				{
					request->complete = true;
					request->tce.set(p);
				}
			}
			else
			{
				std::string idstr = std::to_string(id);
				logger->log(LogLevel::Warn, "RequestProcessor/next", "Unknow request id. " + idstr);
			}

			return true;
		});

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, [wProcessor, logger](Packet_ptr p)
		{
			auto processor = LockOrThrow(wProcessor);

			uint16 id;
			p->stream >> id;

			byte hasValues;
			p->stream >> hasValues;

			if (hasValues == 0)
			{
				SystemRequest_ptr request = processor->freeRequestSlot(id);

				if (request)
				{
					p->metadata["request"] = std::to_string(request->id);
					if (!request->complete)
					{
						request->complete = true;
						request->tce.set(Packet_ptr());
					}
				}
				else
				{
					logger->log(LogLevel::Warn, "RequestProcessor/complete", "Unknow request id " + std::to_string(id));
				}
			}

			return true;
		});

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, [wProcessor, serializer, logger](Packet_ptr p)
		{
			auto processor = LockOrThrow(wProcessor);

			uint16 id;
			p->stream >> id;

			SystemRequest_ptr request = processor->freeRequestSlot(id);

			if (request)
			{
				p->metadata["request"] = std::to_string(request->id);
				std::string msg = serializer.deserializeOne<std::string>(p->stream);
				if (!request->complete)
				{
					request->complete = true;
					std::string message = msg + "(msgId:" + std::to_string(request->operation()) + ")";
					request->tce.set_exception(std::runtime_error(message.c_str()));
				}
			}
			else
			{
				logger->log(LogLevel::Warn, "RequestProcessor/error", "Unknown request id :" + std::to_string(id));
			}

			return true;
		});
	}

	pplx::task<Packet_ptr> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, const StreamWriter& streamWriter, PacketPriority priority, pplx::cancellation_token ct)
	{
		if (peer)
		{
			pplx::task_completion_event<Packet_ptr> tce;
			auto request = reserveRequestSlot(msgId, tce, ct);
			auto wThat = STRM_WEAK_FROM_THIS();

			request->ct = ct;
			if (ct.is_cancelable())
			{
				std::weak_ptr<SystemRequest> wRequest = request;
				request->ct_registration = ct.register_callback([tce, wRequest, wThat]()
				{
					if (auto that = wThat.lock())
					{
						if (auto request = wRequest.lock())
						{
							if (!request->complete)
							{
								that->freeRequestSlot(request->id);
								tce.set_exception(pplx::task_canceled());
							}
						}
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
					stream << request->id;
					if (streamWriter)
					{
						streamWriter(stream);
					}
				}, systemRequestChannelUid, priority, PacketReliability::RELIABLE, metadata);
			}
			catch (const std::exception& ex)
			{
				tce.set_exception(ex);
			}

			return pplx::create_task(tce, ct);
		}
		else
		{
			return pplx::task_from_exception<Packet_ptr>(std::invalid_argument("peer should not be nullptr"));
		}
	}

	SystemRequest_ptr RequestProcessor::reserveRequestSlot(byte msgId, pplx::task_completion_event<Packet_ptr> tce, pplx::cancellation_token ct)
	{
		SystemRequest_ptr request;

		{ // this scope ensures the static id is not used outside the locked mutex.
			std::lock_guard<std::mutex> lock(_mutexPendingRequests);

			static uint16 id = 0;
			// i is used to know if we tested all uint16 available values, whatever the current value of id.
			uint32 i = 0;
			while (i <= 0xffff)
			{
				id++;
				i++;

				if (!mapContains(_pendingRequests, id))
				{
					request = std::make_shared<SystemRequest>(msgId, tce, ct);
					request->id = id;
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
			time(&request->lastRefresh);
			return request;
		}

		throw std::overflow_error("Unable to create a new request: Too many pending requests.");
	}

	SystemRequest_ptr RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		std::lock_guard<std::mutex> lock(_mutexPendingRequests);

		SystemRequest_ptr request;
		if (mapContains(_pendingRequests, requestId))
		{
			request = _pendingRequests[requestId];
			_pendingRequests.erase(requestId);
		}

		return request;
	}
}
