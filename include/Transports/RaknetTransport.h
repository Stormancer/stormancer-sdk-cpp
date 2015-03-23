#pragma once
#include "libs.h"
#include "ITransport.h"
#include "Core/ILogger.h"

namespace Stormancer
{
	class RaknetTransport : public ITransport
	{
	public:
		RaknetTransport(shared_ptr<ILogger*> logger);
		virtual ~RaknetTransport();

	private:
		shared_ptr<ILogger*> _logger;
	};

};
