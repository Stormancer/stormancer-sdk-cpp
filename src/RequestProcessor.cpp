#include "stormancer.h"

namespace Stormancer
{
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
			uint16 id;
			*(p->stream) >> id;

			if (Helpers::mapContains(this->_pendingRequests, id))
			{
				auto request = this->_pendingRequests[id];
				time(&request->lastRefresh);
				request->observer.on_next(p);
				request->tcs.set();
				p->request = request;
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		}));

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, new handlerFunction([this](Packet<>* p) {
			uint16 id;
			*(p->stream) >> id;

			char c;
			*(p->stream) >> c;
			bool hasValues = (c == 1);

			if (Helpers::mapContains(this->_pendingRequests, id))
			{
				auto request = this->_pendingRequests[id];
				p->request = request;

				if (hasValues)
				{
					request->task.then([this, request, id](pplx::task<void> t) {
						freeRequestSlot(request->id);
						request->observer.on_completed();
					});
				}
				else
				{
					freeRequestSlot(request->id);
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
			uint16 id;
			*(p->stream) >> id;

			if (Helpers::mapContains(_pendingRequests, id))
			{
				p->request = _pendingRequests[id];

				freeRequestSlot(id);

				wstring msg;
				ISerializable::deserialize(p->stream, msg);

				exception_ptr eptr = make_exception_ptr(new exception(Helpers::to_string(msg).c_str()));
				p->request->observer.on_error(eptr);
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
		auto tce = new pplx::task_completion_event<Packet<>*>();
		auto observer = rx::make_observer<Packet<>*>([tce](Packet<>* p) {
			tce->set(p);
		}, [tce](exception_ptr ex) {
			tce->set_exception(ex);
		});
		auto request = reserveRequestSlot(observer.as_dynamic());

		peer->sendSystem(msgId, [request, &writer](bytestream* bs) {
			*bs << request->id;
			writer(bs);
		});

		auto task = pplx::create_task(*tce);
		task.then([this, tce](pplx::task<Packet<>*> t) {
			if (tce)
			{
				delete tce;
			}
		});

		return task;
	}

	shared_ptr<Request> RequestProcessor::reserveRequestSlot(PacketObserver&& observer)
	{
		static uint16 id = 0;
		while (id < UINT16_MAX)
		{
			if (!Helpers::mapContains(_pendingRequests, id))
			{
				shared_ptr<Request> request( new Request(std::move(observer)) );
				time(&request->lastRefresh);
				request->id = id;
				_pendingRequests[id] = request;
				return request;
			}
			id++;
		}
		_logger->log(LogLevel::Error, L"", L"Unable to create a new request: Too many pending requests.", L"");
		throw exception("Unable to create a new request: Too many pending requests.");
	}

	bool RequestProcessor::freeRequestSlot(uint16 requestId)
	{
		if (Helpers::mapContains(this->_pendingRequests, requestId))
		{
			this->_pendingRequests.erase(requestId);
			return true;
		}
		return false;
	}
};
