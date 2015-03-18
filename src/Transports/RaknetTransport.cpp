#include "Transports/RaknetTransport.h"

namespace Stormancer
{
	RaknetTransport::RaknetTransport(ILogger& logger)
		: _logger(logger)
	{
	}

	RaknetTransport::~RaknetTransport()
	{
	}
};
