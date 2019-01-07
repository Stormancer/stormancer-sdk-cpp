#include "stormancer/stdafx.h"
#include "stormancer/Shutdown.h"

namespace Stormancer
{
	Shutdown Shutdown::_instance = Shutdown();
	Shutdown::~Shutdown()
	{
		_cts.cancel();
	}

	pplx::cancellation_token Shutdown::getShutdownToken()
	{
		return _cts.get_token();
	}
}