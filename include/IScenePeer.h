#pragma once
#include "headers.h"

namespace Stormancer
{
	class IScenePeer
	{
	public:
		IScenePeer();
		virtual ~IScenePeer();

	public:
		virtual void send(wstring& routeName, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
	};
};