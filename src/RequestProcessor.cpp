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
				throw string("Can only add handler before 'RegisterProcessor' is called.");
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
			config->addProcessor(it.first, [it](Packet<>* p) {
				RequestContext context(p);
				it.second(&context).then([&context, &p](pplx::task<void> task) {
					if (!context.isComplete())
					{
						if (false)
						{
							context.error([&p](bytestream* stream) {
								auto MsgPackSrlz = dynamic_cast<MsgPackSerializer*>(p->serializer());
								string msg = "An error occured on the server.";
								MsgPackSrlz->serialize(msg, stream);
							});
						}
						else
						{
							context.complete();
						}
					}
				});
				return true;
			});
		}

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, [this](Packet<>* p) {
			byte temp[2];
			p->stream->readsome((char*)temp, 2);
			uint16 id = temp[0] * 256 + temp[1];

			if (Helpers::mapContains(_pendingRequests, id))
			{
				Request* request = _pendingRequests[id];
				time(&request->lastRefresh);
				request->observer->on_next(p);
				request->tcs.set();
				p->setMetadata(L"request", request);
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		});

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, [this](Packet<>* p) {
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
						return request->observer->on_completed();
					});
				}
				else
				{
					request->observer->on_completed();
				}
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		});

		config->addProcessor((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, [this](Packet<>* p) {
			byte temp[2];
			p->stream->readsome((char*)temp, 2);
			uint16 id = temp[0] * 256 + temp[1];

			if (Helpers::mapContains(_pendingRequests, id))
			{
				Request* request = _pendingRequests[id];
				p->setMetadata(L"request", request);

				auto it = _pendingRequests.find(id);
				_pendingRequests.erase(it);

				auto MsgPckSrlz = dynamic_cast<MsgPackSerializer*>(p->serializer());
				wstring msg;
				MsgPckSrlz->deserialize(p->stream, msg);

				exception_ptr eptr = make_exception_ptr(new exception(Helpers::to_string(msg).c_str()));
				request->observer->on_error(eptr);
			}
			else
			{
				_logger->log(LogLevel::Trace, L"", L"Unknow request id.", L"");
			}

			return true;
		});
	}

	pplx::task<Packet<>*> RequestProcessor::sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer)
	{
		//auto tcs = task_completion_event<Packet<>>();
		//auto request = reserveRequestSlot(rx::observer<>);
		// TODO
		throw string("Not implem");
	}

	pplx::task<Packet<>*> RequestProcessor::sendSceneRequest(IConnection* peer, byte sceneId, uint16 routeId, function<void(bytestream*)> writer)
	{
		throw string("Not implem");
		/*return rx::observable<>::create<Packet<>>([](rx::subscriber<Packet<>> dest) {
			//auto request = reserveRequestSlot(dest);
			// TODO
		});*/
	}

	RequestProcessor::Request* RequestProcessor::reserveRequestSlot(rx::observer<Packet<>*>* observer)
	{
		static uint16 id = 0;
		while (id < UINT16_MAX)
		{
			if (!Helpers::mapContains(_pendingRequests, id))
			{
				_pendingRequests[id] = new Request();
				Request* request = _pendingRequests[id];
				time(&request->lastRefresh);
				request->id = id;
				request->observer = observer;
				return request;
			}
			id++;
		}
		_logger->log(LogLevel::Error, L"", L"Unable to create a new request: Too many pending requests.", L"");
		throw string("Unable to create a new request: Too many pending requests.");
	}
};
