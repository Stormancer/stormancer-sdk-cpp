#include "libs.h"
#include "Transports/RaknetTransport.h"

namespace Stormancer
{
	RaknetTransport::RaknetTransport(shared_ptr<ILogger*> logger)
		: _logger(logger)
	{
	}

	RaknetTransport::~RaknetTransport()
	{
	}
};
