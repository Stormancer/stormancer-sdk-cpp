#include "stormancer.h"

namespace Stormancer
{
	RakNetTransport::RakNetTransport(ILogger* logger)
		: _logger(logger)
	{
		ITransport::_name = L"raknet";
	}

	RakNetTransport::~RakNetTransport()
	{
	}

	pplx::task<void> RakNetTransport::start(wstring type, IConnectionManager* handler, pplx::cancellation_token token, uint16 serverPort, uint16 maxConnections)
	{
		throw string("no implem");
		//return pplx::create_task([]() {});
	}

	void RakNetTransport::run(pplx::cancellation_token token, pplx::task_completion_event<bool> startupTcs, uint16 serverPort, uint16 maxConnections)
	{
		throw string("no implem");
	}

	pplx::task<IConnection*> RakNetTransport::connect(wstring endpoint)
	{
		throw string("no implem");
	}

	void RakNetTransport::onConnectionReceived(uint64 p)
	{
		_id = p;
	}
};
