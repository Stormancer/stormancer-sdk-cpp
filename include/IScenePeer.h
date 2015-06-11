#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Remote scene
	class IScenePeer
	{
	public:
	
		/// Constructor.
		IScenePeer();
		
		/// Destructor.
		virtual ~IScenePeer();

	public:
	
		/// Sends a message to the remote scene.
		/// \param routeName Route name.
		/// \param writer function where we write the message in the stream.
		/// \param priority Priority of the message in the network.
		/// \param reliability Reliability behavior of the message in the network.
		virtual void send(wstring& routeName, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
	};
};
