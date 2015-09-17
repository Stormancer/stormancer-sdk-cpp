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
				it.second(&context).then([&context, &p](pplx::task<void> task) {
					if (!context.isComplete())
					{
						if (false)
						{
							context.error([&p](bytestream* stream) {
								msgpack::packer<bytestream> pk(stream);
								pk.pack(std::string("An error occured on the server."));
							});
						}
						else
						{
							context.complete();
						}
					}
				});
				return true;
			}));
		}

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			if (mapContains(this->_pendingRequests, id))
			{
				auto request = this->_pendingRequests[id];
				time(&request->lastRefresh);
				request->observer.on_next(p);
				request->observer.on_completed();
				p->request = request;
				freeRequestSlot(request->id);
			}
			else
			{
				_logger->log(LogLevel::Trace, "", "Unknow request id.", to_string(id));
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			char c;
			*(p->stream) >> c;
			bool hasValues = (c == 1);

			if (!hasValues)
			{
				if (mapContains(this->_pendingRequests, id))
				{
					auto request = this->_pendingRequests[id];
					p->request = request;

					request->observer.on_completed();
					freeRequestSlot(request->id);
				}
				else
				{
					_logger->log(LogLevel::Trace, "", "Unknow request id.", to_string(id));
				}
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, new handlerFunction([this](Packet_ptr p) {
			uint16 id;
			*(p->stream) >> id;

			if (mapContains(_pendingRequests, id))
			{
				p->request = _pendingRequests[id];

				std::string buf;
				*p->stream >> buf;
				std::stringstream ss;
				for (uint32 i = 0; i < buf.size(); i++)
				{
					ss << (int)(buf[i]) << ",";
				}
				msgpack::unpacked result;
				msgpack::unpack(result, buf.data(), buf.size());
				msgpack::object deserialized = result.get();
				std::string msg;
				deserialized.convert(&msg);

				auto eptr = std::make_exception_ptr(new std::exception());//(msg));
				p->request->observer.on_error(eptr);

				freeRequestSlot(id);
			}
			else
			{
				_logger->log(LogLevel::Trace, "", "Unknow request id.", to_string(id));
			}

			return true;
		}));
	}

	pplx::task<Packet_ptr> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		auto tce = new pplx::task_completion_event<Packet_ptr>();
		auto observer = rxcpp::make_observer<Packet_ptr>([tce](Packet_ptr p) {
			tce->set(p);
		}, [tce](std::exception_ptr ex) {
			try
			{
				std::rethrow_exception(ex);
			}
			catch (const std::exception& e)
			{
				tce->set_exception<std::exception>(e);
			}
		});
		auto request = reserveRequestSlot(observer.as_dynamic());

		peer->sendSystem(msgId, [request, &writer](bytestream* stream) {
			*stream << request->id;
			writer(stream);
		}, priority);

		auto task = pplx::create_task(*tce);
		return task.then([tce](pplx::task<Packet_ptr> t) {
			if (tce)
			{
				delete tce;
			}
			try
			{
				return t.get();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string(e.what()) + "System request failed.");
			}
		});
	}

	Request_ptr RequestProcessor::reserveRequestSlot(PacketObserver&& observer)
	{
		static uint16 id = 0;
		int32 i = 0;
		while (i <= 0xffff)
		{
			if (!mapContains(_pendingRequests, id))
			{
				Request_ptr request(new Request(std::move(observer)));
				time(&request->lastRefresh);
				request->id = id;
				_pendingRequests[id] = request;
				return request;
			}
			id++;
			i++;
		}
		_logger->log(LogLevel::Error, "", "Unable to create a new request: Too many pending requests.", "");
		throw std::overflow_error("Unable to create a new request: Too many pending requests.");
	}

	bool RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		if (mapContains(this->_pendingRequests, requestId))
		{
			this->_pendingRequests.erase(requestId);
			return true;
		}
		return false;
	}
};
