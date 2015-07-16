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
			config.addProcessor(it.first, new handlerFunction([it](std::shared_ptr<Packet<>> p) {
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

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, new handlerFunction([this](std::shared_ptr<Packet<>> p) {
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
				_logger->log(LogLevel::Trace, "", "Unknow request id.", "");
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, new handlerFunction([this](std::shared_ptr<Packet<>> p) {
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
					_logger->log(LogLevel::Trace, "", "Unknow request id.", "");
				}
			}

			return true;
		}));

		config.addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, new handlerFunction([this](std::shared_ptr<Packet<>> p) {
			_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "ID_REQUEST_RESPONSE_ERROR", "");
			uint16 id;
			*(p->stream) >> id;
			_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R1", "");

			if (mapContains(_pendingRequests, id))
			{
				p->request = _pendingRequests[id];
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R2", "");

				std::string buf;
				*p->stream >> buf;
				std::stringstream ss;
				for (int i = 0; i < buf.size(); i++)
				{
					ss << (int)(buf[i]) << ",";
				}
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R3", stringFormat(buf, " ", ss.str()));
				msgpack::unpacked result;
				msgpack::unpack(result, buf.data(), buf.size());
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R4", "");
				msgpack::object deserialized = result.get();
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R5", "");
				std::string msg;
				deserialized.convert(&msg);
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R6", "");

				auto eptr = std::make_exception_ptr(new std::runtime_error(msg));
				p->request->observer.on_error(eptr);

				freeRequestSlot(id);
				_logger->log(LogLevel::Trace, "RequestProcessor::registerProcessor::lambda", "R7", "");
			}
			else
			{
				_logger->log(LogLevel::Trace, "", "Unknow request id.", "");
			}

			return true;
		}));
	}

	pplx::task<std::shared_ptr<Packet<>>> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, std::function<void(bytestream*)> writer)
	{
		auto tce = new pplx::task_completion_event<std::shared_ptr<Packet<>>>();
		auto observer = rxcpp::make_observer<std::shared_ptr<Packet<>>>([tce](std::shared_ptr<Packet<>> p) {
			tce->set(p);
		}, [tce](std::exception_ptr ex) {
			tce->set_exception(ex);
		});
		auto request = reserveRequestSlot(observer.as_dynamic());

		peer->sendSystem(msgId, [request, &writer](bytestream* stream) {
			*stream << request->id;
			writer(stream);
		});

		auto task = pplx::create_task(*tce);
		task.then([tce](pplx::task<std::shared_ptr<Packet<>>> t) {
			if (tce)
			{
				delete tce;
			}
		});

		return task;
	}

	std::shared_ptr<Request> RequestProcessor::reserveRequestSlot(PacketObserver&& observer)
	{
		static uint16 id = 0;
		while (id < 0xffff)
		{
			if (!mapContains(_pendingRequests, id))
			{
				std::shared_ptr<Request> request(new Request(std::move(observer)));
				time(&request->lastRefresh);
				request->id = id;
				_pendingRequests[id] = request;
				return request;
			}
			id++;
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
