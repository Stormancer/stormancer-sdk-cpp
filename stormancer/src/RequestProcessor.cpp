#include "stdafx.h"
#include "RequestProcessor.h"
#include "MessageIDTypes.h"

namespace Stormancer
{
	void RequestProcessor::Initialize(std::shared_ptr<RequestProcessor> processor, std::vector<std::shared_ptr<IRequestModule>> modules)
	{
		RequestModuleBuilder builder(processor->addSystemRequestHandler);
		for (size_t i = 0; i < modules.size(); i++)
		{
			auto module = modules[i];
			module->registerModule(&builder);
		}
	}

	RequestProcessor::RequestProcessor(std::shared_ptr<ILogger> logger)
		: _logger(logger)
	{
		addSystemRequestHandler = [this](byte msgId, std::function<pplx::task<void>(RequestContext*)> handler)
		{
			if (_isRegistered)
			{
				throw std::runtime_error("Can only add handler before 'RegisterProcessor' is called.");
			}
			_handlers[msgId] = handler;
		};
	}

	RequestProcessor::~RequestProcessor()
	{
	}

	void RequestProcessor::registerProcessor(PacketProcessorConfig& config)
	{
		_isRegistered = true;

		config.addProcessor((byte)MessageIDTypes::ID_SYSTEM_REQUEST, new handlerFunction([this](Packet_ptr  p) {
			Stormancer::byte sysRequestId;
			*(p->stream) >> sysRequestId;
			std::shared_ptr<RequestContext> context = std::make_shared<RequestContext>(p);
			auto it = _handlers.find(sysRequestId);

			if (it == _handlers.end())
			{
				context->error([](bytestream* stream) {
					msgpack::packer<bytestream> pk(stream);
					pk.pack(std::string("No system request handler found."));
				});
				return true;
			}
			auto ctxPtr = context.get();
			std::function<pplx::task<void>(RequestContext*)> handler = it->second;
			invokeWrapping(handler, ctxPtr).then([context, p](pplx::task<void> t) {
				//Clean the packet to deallocate resources
				p->clean();
				if (!context->isComplete())
				{
					// task faulted
					try
					{
						t.get();
					}
					catch (const std::exception& ex)
					{
						context->error([ex](bytestream* stream) {
							msgpack::packer<bytestream> pk(stream);
							pk.pack(std::string("An error occured on the server. ") + ex.what());
						});
						return;
					}

					// task completed
					context->complete();
				}

			});

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			SystemRequest_ptr request = freeRequestSlot(id);

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
				ILogger::instance()->log(LogLevel::Warn, "RequestProcessor/next", "Unknow request id. " + idstr);
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			char hasValues;
			*(p->stream) >> hasValues;

			if (hasValues == 0)
			{
				SystemRequest_ptr request = freeRequestSlot(id);

				if (request)
				{
					p->metadata["request"] = std::to_string(request->id);
					if (!request->complete)
					{
						request->complete = true;
						request->tce.set(nullptr);
					}
				}
				else
				{
					ILogger::instance()->log(LogLevel::Warn, "RequestProcessor/complete", "Unknow request id " + to_string(id));
				}
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			SystemRequest_ptr request = freeRequestSlot(id);

			if (request)
			{
				p->metadata["request"] = std::to_string(request->id);
				std::string buf;
				*p->stream >> buf;
				msgpack::unpacked result;
				msgpack::unpack(result, buf.data(), buf.size());
				msgpack::object deserialized = result.get();
				std::string msg;
				deserialized.convert(&msg);
				if (!request->complete)
				{
					request->complete = true;
					request->tce.set_exception(std::runtime_error(msg+"(msgId:"+std::to_string(request->operation())+ ")"));
				}
			}
			else
			{
				ILogger::instance()->log(LogLevel::Warn, "RequestProcessor/error", "Unknown request id :" + to_string(id));
			}

			return true;
		}));
	}

	pplx::task<Packet_ptr> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		auto tce = pplx::task_completion_event<Packet_ptr>();
		if (peer)
		{
			auto request = reserveRequestSlot(msgId,tce);
			
			try
			{
				peer->sendSystem((byte)MessageIDTypes::ID_SYSTEM_REQUEST, [request, &writer, msgId](bytestream* stream) {
					*stream << msgId;
					*stream << request->id;
					writer(stream);
				}, priority);
			}
			catch (const std::exception& ex)
			{
				tce.set_exception(ex);
			}
		}
		else
		{
			tce.set_exception(std::invalid_argument("peer should not be nullptr"));
		}
		return pplx::create_task(tce);
	}

	SystemRequest_ptr RequestProcessor::reserveRequestSlot(byte msgId, pplx::task_completion_event<Packet_ptr> tce)
	{
		SystemRequest_ptr request;
		uint16 selectedId;

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
					request = std::make_shared<SystemRequest>(msgId,tce);
					break;
				}
			}
			selectedId = id;
		}

		if (request)
		{
			time(&request->lastRefresh);
			request->id = selectedId;
			_pendingRequests[selectedId] = request;
			return request;
		}

		throw std::overflow_error("Unable to create a new request: Too many pending requests.");
	}

	SystemRequest_ptr RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		_mutexPendingRequests.lock();

		SystemRequest_ptr request;
		if (mapContains(_pendingRequests, requestId))
		{
			request = _pendingRequests[requestId];
			_pendingRequests.erase(requestId);
		}

		_mutexPendingRequests.unlock();

		return request;
	}
};
