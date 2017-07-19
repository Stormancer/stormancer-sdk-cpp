#pragma once

#include "headers.h"
#include "msgpack.hpp"
#include "basic_bytestream.h"

namespace Stormancer
{
	class Serializer
	{
	public:

		/// Try to deserialize a type from the provided stream and advances the stream.
		/// \param s Source stream
		/// \return the object deserialized from the stream.
		template<typename T>
		T deserialize(bytestream* stream)
		{
			auto g = stream->tellg();
			std::string buffer;
			*stream >> buffer;
			msgpack::unpacked unp;
			auto readOffset = msgpack::unpack(unp, buffer.data(), buffer.size());
			g += readOffset;
			stream->seekg(g);
			T result;
			unp.get().convert(&result);
			return result;
		}

		template<typename T>
		void serialize(T data, bytestream* s)
		{
			msgpack::packer<bytestream> pk(s);
			pk.pack(data);
		}
	};
}
