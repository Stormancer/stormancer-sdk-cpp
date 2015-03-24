
#include "Processors/RequestProcessor.h"
#include "Helpers.h"

namespace Stormancer
{
	RequestProcessor::RequestProcessor(shared_ptr<ILogger*> logger, vector<shared_ptr<IRequestModule*>> modules)
		: _logger(logger)
	{
		auto addSystemRequestHandler = [&]() {
			
		};

		auto builder = RequestModuleBuilder(addSystemRequestHandler);
		for (int i = 0; i < modules.size(); i++)
		{
			auto module = modules[i];
			(*module)->registerModule(builder);
		}
	}

	RequestProcessor::~RequestProcessor()
	{
	}

	void RequestProcessor::registerProcessor(PacketProcessorConfig config)
	{
		// TODO
	}

	void RequestProcessor::addSystemRequestHandler(byte msgId, function<pplx::task<void>(RequestContext)> handler)
	{
		if (_isRegistered)
		{
			throw new exception(Helpers::StringFormat() << "Can only add handler before 'RegisterProcessor' is called.");
		}
		_handlers[msgId] = handler;
	}
};
