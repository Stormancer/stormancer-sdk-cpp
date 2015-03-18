#pragma once
#include "stdafx.h"
#include "ITransport.h"
#include "Core/ILogger.h"

namespace Stormancer
{
	class RaknetTransport : public ITransport
	{
	public:
		RaknetTransport(ILogger& logger);
		virtual ~RaknetTransport();

	private:
		ILogger& _logger;
	};

};
