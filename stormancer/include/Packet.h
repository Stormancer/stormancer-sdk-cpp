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

#pragma region public_methods

		/// Constructor
		/// \param source Generic source of the packets.
		/// \param stream Data stream attached to the packet.
		Packet(std::shared_ptr<T> source, bytestream* stream)
			: connection(source)
			, stream(stream)
		{
		}

		/// Constructor
		/// \param source Generic source of the packets.
		/// \param stream Data stream attached to the packet.
		/// \param metadata Metadata attached to this packet.
		Packet(std::shared_ptr<T> source, bytestream* stream, const std::map<std::string, std::string>& metadata)
			: connection(source)
			, stream(stream)
			, metadata(metadata)
		{
		}

		/// Destructor
		virtual ~Packet()
		{
			clean();
		}

		void clean()
		{
			cleanup();
			cleanup.clear();
		}

		/// Read a msgpack object
		template<typename TOut>
		TOut readObject()
		{
			auto g = stream->tellg();
			std::string buffer;
			*stream >> buffer;
			msgpack::unpacked unp;
			auto readOffset = msgpack::unpack(unp, buffer.data(), buffer.size());
			g += readOffset;
			stream->seekg(g);
			TOut result;
			unp.get().convert(&result);
			return result;
		}

#pragma endregion

#pragma region public_members

		/// Source
		std::shared_ptr<T> connection = nullptr;

		/// Data stream
		bytestream* stream = nullptr;

		/// Clean-up operations
		Action<> cleanup;

		/// Metadata
		std::map<std::string, std::string> metadata;

#pragma endregion

	private:

#pragma region private_methods

		/// Copy constructor deleted.
		Packet(const Packet<T>&) = delete;

		/// Copy operator deleted.
		Packet<T>& operator=(const Packet<T>&) = delete;

#pragma endregion
	};

	using Packet_ptr = std::shared_ptr<Packet<>>;
	using Packetisp_ptr = std::shared_ptr<Packet<IScenePeer>>;
	using PacketObservable = rxcpp::observable<Packet_ptr>;
	using handlerFunction = std::function<bool(Packet_ptr)>;
	using processorFunction = std::function<bool(byte, Packet_ptr)>;
};
