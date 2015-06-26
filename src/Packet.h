#pragma once
#include "headers.h"
#include "IConnection.h"
#include "Helpers.h"
#include "Action.h"

namespace Stormancer
{
	class Request;

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
		anyMap metadata()
		{
			return _metadata;
		}

		/// Set a pointer to a data in the metadata.
		template<typename TData>
		void setMetadata(std::string key, TData* data)
		{
			_metadata[key] = static_cast<void*>(data);
		}

		/// Get a pointer to a data of the packet metadatas.
		template<typename TData>
		TData* getMetadata(std::string key)
		{
			return (TData*)_metadata[key];
		}

		/// Remove a data from the metadatas.
		void removeMetadata(std::string key)
		{
			auto it = _metadata.find(key);
			_metadata.erase(it);
		}

	public:
	
		/// source.
		T* connection = nullptr;
		
		/// data stream.
		bytestream* stream = nullptr;

		/// Attached request.
		std::shared_ptr<Request> request = nullptr;

		/// Clean-up operations.
		Action<> cleanup;

	private:
	
		/// metadatas.
		anyMap _metadata;
	};
};
