
#include "ITransport.h"

namespace Stormancer
{
	ITransport::ITransport()
		: _isRunning(false)
	{
	}

	ITransport::~ITransport()
	{
	}

	bool ITransport::isRunning()
	{
		return _isRunning;
	}

	wstring ITransport::name()
	{
		return _name;
	}

	uint64 ITransport::id()
	{
		return _id;
	}
};
