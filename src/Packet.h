#pragma once
#include "headers.h"
#include "IConnection.h"
#include "Helpers.h"
#include "Action.h"
#include "IScenePeer.h"

namespace Stormancer
{
	/// Expose a stream for reading data received from the network.
	template<typename T = IConnection>
	class Packet
	{
	public:
	
		/// Constructor.
		/// \param source Generic source of the packets.
		/// \param stream Data stream attached to the packet.
		Packet(T* source, bytestream* stream)
			: connection(source),
			stream(stream)
		{
		}

		/// Constructor.
		/// \param source Generic source of the packets.
		/// \param stream Data stream attached to the packet.
		/// \param metadata Metadata attached to this packet.
		Packet(T* source, bytestream* stream, anyMap& metadata)
			: connection(source),
			stream(stream),
			_metadata(metadata)
		{
		}
		
		/// Copy constructor deleted.
		Packet(const Packet<T>&) = delete;
		
		/// Copy operator deleted.
		Packet<T>& operator=(const Packet<T>&) = delete;

		/// Destructor.
		virtual ~Packet()
		{
			cleanup();
		}

	public:
	
		/// Returns a copy of the packet metadatas.
		anyMap& metadata()
		{
			return _metadata;
		}

	public:
	
		/// source.
		T* connection = nullptr;
		
		/// data stream.
		bytestream* stream = nullptr;
		
		/// Clean-up operations.
		Action<> cleanup;

	private:
	
		/// metadatas.
		anyMap _metadata;
	};
};
