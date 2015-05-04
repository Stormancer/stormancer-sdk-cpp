#include "stormancer.h"

namespace Stormancer
{
	RequestProcessor::Request::Request(PacketObserver&& observer)
		: observer(observer),
		task(std::move(create_task(tcs)))
	{
	}

	RequestProcessor::Request::~Request()
	{
	}

	RequestProcessor::RequestProcessor(ILogger* logger, vector<IRequestModule*> modules)
		: _logger(logger)
	{
		auto builder = new RequestModuleBuilder(RequestProcessor::addSystemRequestHandler);
		for (size_t i = 0; i < modules.size(); i++)
		{
			auto module = modules[i];
			module->registerModule(builder);
		}

		addSystemRequestHandler = [this](byte msgId, function<pplx::task<void>(RequestContext*)> handler)
		{
			if (_isRegistered)
			{
				throw exception("Can only add handler before 'RegisterProcessor' is called.");
			}
			_handlers[msgId] = handler;
		};
	}

	RequestProcessor::~RequestProcessor()
	{
	}

	void RequestProcessor::registerProcessor(PacketProcessorConfig* config)
	{
		_isRegistered = true;

		for (auto it : _handlers)
		{
			config->addProcessor(it.first, new handlerFunction([it](Packet<>* p) {
				RequestContext context(p);
				it.second(&context).then([&context, &p](pplx::task<void> task) {
					if (!context.isComplete())
					{
						if (false)
						{
							context.error([&p](bytestream* stream) {
								string msg = "An error occured on the server.";
								ISerializable::serialize(msg, stream);
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

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, new handlerFunction([this](Packet<>* p) {
			byte temp[2];
			auto str = p->stream->str();
			p->stream->readsome((char*)temp, 2);
			uint16 id = temp[0] * 256 + temp[1];

			if (Helpers::mapContains(_pendingRequests, id))
			{
				Request* request = _pendingRequests[id];
				time(&request->lastRefresh);
				request->observer.on_next(p);
				request->tcs.set();
				p->setMetadata(L"request", request);
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		}));

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, new handlerFunction([this](Packet<>* p) {
			byte temp[2];
			p->stream->readsome((char*)temp, 2);
			uint16 id = temp[0] * 256 + temp[1];

			char c;
			p->stream->readsome(&c, 1);
			bool hasValues = (c == 1);

			if (Helpers::mapContains(_pendingRequests, id))
			{
				Request* request = _pendingRequests[id];
				p->setMetadata(L"request", request);

				auto it = _pendingRequests.find(id);
				_pendingRequests.erase(it);


				if (hasValues)
				{
					request->task.then([request](pplx::task<void> t) {
						return request->observer.on_completed();
					});
				}
				else
				{
					request->observer.on_completed();
				}
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		}));

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, new handlerFunction([this](Packet<>* p) {
			byte temp[2];
			p->stream->readsome((char*)temp, 2);
			uint16 id = temp[0] * 256 + temp[1];

			if (Helpers::mapContains(_pendingRequests, id))
			{
				Request* request = _pendingRequests[id];
				p->setMetadata(L"request", request);

				auto it = _pendingRequests.find(id);
				_pendingRequests.erase(it);

				wstring msg;
				ISerializable::deserialize(p->stream, msg);

				exception_ptr eptr = make_exception_ptr(new exception(Helpers::to_string(msg).c_str()));
				request->observer.on_error(eptr);
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		}));
	}

	pplx::task<Packet<>*> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer)
	{
		auto tce = pplx::task_completion_event<Packet<>*>();
		auto observer = rx::make_observer<Packet<>*>([&tce](Packet<>* p) {
			tce.set(p);
		}, [&tce](exception_ptr ex) {
			tce.set_exception(ex);
		});
		auto request = reserveRequestSlot(observer.as_dynamic());

		peer->sendSystem(msgId, [&request, &writer](bytestream* bs) {
			*bs << request->id;
			writer(bs);
		});

		auto task = pplx::create_task(tce);
		task.then([this, request](pplx::task<Packet<>*> t) {
			if (Helpers::mapContains(this->_pendingRequests, request->id))
			{
				auto r = this->_pendingRequests[request->id];
				if (r == request)
				{
					this->_pendingRequests.erase(request->id);
				}
			}
		});

		return task;
	}

	RequestProcessor::Request* RequestProcessor::reserveRequestSlot(PacketObserver&& observer)
	{
		static uint16 id = 0;
		while (id < UINT16_MAX)
		{
			if (!Helpers::mapContains(_pendingRequests, id))
			{
				_pendingRequests[id] = new Request(std::move(observer));
				Request* request = _pendingRequests[id];
				time(&request->lastRefresh);
				request->id = id;
				return request; 
			}
			id++;
		}
		_logger->log(LogLevel::Error, L"", L"Unable to create a new request: Too many pending requests.", L"");
		throw exception("Unable to create a new request: Too many pending requests.");
	}
};
