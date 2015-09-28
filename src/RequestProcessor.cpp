#include "stormancer.h"

namespace Stormancer
{
	RequestProcessor::RequestProcessor(ILogger* logger, std::vector<IRequestModule*> modules)
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

		RequestModuleBuilder builder(addSystemRequestHandler);
		for (size_t i = 0; i < modules.size(); i++)
		{
			auto module = modules[i];
			module->registerModule(&builder);
		}
	}

	RequestProcessor::~RequestProcessor()
	{
	}

	void RequestProcessor::registerProcessor(PacketProcessorConfig& config)
	{
		_isRegistered = true;

		for (auto it : _handlers)
		{
			config.addProcessor(it.first, new handlerFunction([it](Packet_ptr p) {
				RequestContext context(p);
				it.second(&context).then([&context, p](pplx::task<void> t) {
					if (!context.isComplete())
					{
						// task faulted
						try
						{
							t.wait();
						}
						catch (const std::exception& e)
						{
							context.error([p, e](bytestream* stream) {
								msgpack::packer<bytestream> pk(stream);
								pk.pack(std::string("An error occured on the server. ") + e.what());
							});
							return;
						}

						// task completed
						context.complete();
					}
				});
				return true;
			}));
		}

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			SystemRequest_ptr request = freeRequestSlot(id);

			if (request)
			{
				p->metadata()["request"] = (void*)request.get();
				time(&request->lastRefresh);
				request->tce.set(p);
			}
			else
			{
				_logger->log(LogLevel::Warn, "RequestProcessor (message)", "Unknow request id.", to_string(id));
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
					p->metadata()["request"] = (void*)request.get();
					request->tce.set(nullptr);
				}
				else
				{
					_logger->log(LogLevel::Warn, "RequestProcessor (complete)", "Unknow request id.", to_string(id));
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
				p->metadata()["request"] = (void*)request.get();
				std::string buf;
				*p->stream >> buf;
				msgpack::unpacked result;
				msgpack::unpack(result, buf.data(), buf.size());
				msgpack::object deserialized = result.get();
				std::string msg;
				deserialized.convert(&msg);
				request->tce.set_exception<std::exception>(std::runtime_error(msg));
			}
			else
			{
				_logger->log(LogLevel::Warn, "RequestProcessor (error)", "Unknow request id.", to_string(id));
			}

			return true;
		}));
	}

	pplx::task<Packet_ptr> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		auto tce = pplx::task_completion_event<Packet_ptr>();
		auto request = reserveRequestSlot(tce);

		peer->sendSystem((byte)MessageIDTypes::ID_SYSTEM_REQUEST, [request, &writer, msgId](bytestream* stream) {
			*stream << msgId;
			*stream << request->id;
			writer(stream);
		}, priority);

		return pplx::create_task(tce);
	}

	SystemRequest_ptr RequestProcessor::reserveRequestSlot(pplx::task_completion_event<Packet_ptr> tce)
	{
		_mutexPendingRequests.lock();

		static uint16 id = 0;
		// i is used to know if we tested all uint16 available values, whatever the current value of id.
		uint32 i = 0;
		while (i <= 0xffff)
		{
			SystemRequest_ptr request;
			if (!mapContains(_pendingRequests, id))
			{
				request = SystemRequest_ptr(new SystemRequest(tce));
				time(&request->lastRefresh);
				request->id = id;
				_pendingRequests[id] = request;
				return request;
			}

			id++;
			i++;
		}

		_mutexPendingRequests.unlock();

		throw std::overflow_error("Unable to create a new request: Too many pending requests.");
	}

	SystemRequest_ptr RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		//std::stringstream ss;
		//ss << std::this_thread::get_id();
		//_logger->log(std::string("#5 lock ") + ss.str());
		_mutexPendingRequests.lock();
		//_logger->log("#5 exec");
		SystemRequest_ptr request;
		if (mapContains(_pendingRequests, requestId))
		{
			request = _pendingRequests[requestId];
			_pendingRequests.erase(requestId);
		}
		//_logger->log("#5 unlock");
		_mutexPendingRequests.unlock();
		//_logger->log("#5 unlocked");

		return request;
	}
};
