#pragma once
#include "stdafx.h"
#include "IPacketDispatcher.h"

namespace Stormancer
{
	class DefaultPacketDispatcher : public IPacketDispatcher
	{
	public:
		DefaultPacketDispatcher();
		virtual ~DefaultPacketDispatcher();
	};
};
