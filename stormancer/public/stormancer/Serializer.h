#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	class Serializer
	{
	public:

#pragma region public_methods

		// SERIALISE

		template<typename T, typename... Args>
		void serialize(obytestream* stream, const T& value, const Args&... args) const
		{
			msgpack::pack(stream, value);
			serialize(stream, args...);
		}

		void serialize(obytestream*) const;

		// DESERIALISE

		/// Try to deserialize a type from the provided stream and advances the stream.
		/// \param s Source stream
		/// \return the object deserialized from the stream.
		template<typename TOutput>
		TOutput deserializeOne(ibytestream* stream) const
		{
			TOutput result;
			deserialize(stream, result);
			return result;
		}

		/// Try to deserialize a type from the provided stream and advances the stream.
		/// \param byteArray Source bytes array
		/// \return the object deserialized from the stream.
		template<typename TOutput>
		TOutput deserializeOne(const byte* data, const uint64 dataSize, uint64* readOffset = nullptr) const
		{
			TOutput result;
			deserialize(data, dataSize, result, readOffset);
			return result;
		}
		
		/// Try to deserialize a type from the provided stream and advances the stream.
		/// \param s Source stream
		/// \return the object deserialized from the stream.
		template<typename... Args>
		void deserialize(ibytestream* stream, Args&... args) const
		{
			auto g = stream->tellg();

			std::vector<byte> buffer;
			(*stream) >> buffer;

			uint64 readOffset = 0;
			UnstackAndDeserialize<Args...>(buffer.data(), buffer.size(), &readOffset, args...);

			g += readOffset;
			stream->seekg(g);
		}

		/// Try to deserialize a type from the provided stream and advances the stream.
		/// \param byteArray Source bytes array
		/// \return the object deserialized from the stream.
		template<typename TOutput>
		void deserialize(const byte* data, const uint64 dataSize, TOutput& value, uint64* readOffset = nullptr) const
		{
			msgpack::unpacked unp;
			uint64 readOffset2 = msgpack::unpack(unp, (char*)data, (std::size_t)dataSize);
			if (readOffset)
			{
				(*readOffset) += readOffset2;
			}
			unp.get().convert(&value);
		}

#pragma endregion

	private:

#pragma region private_methods

		template<typename T, typename... Args>
		void UnstackAndDeserialize(const byte* data, const uint64 dataSize, uint64* readOffset, T& value, Args&... args) const
		{
			const byte* data2 = data + (*readOffset);
			uint64 dataSize2 = dataSize - (*readOffset);
			deserialize<T>(data2, dataSize2, value, readOffset);
			UnstackAndDeserialize(data, dataSize, readOffset, args...);
		}

		void UnstackAndDeserialize(const byte*, const uint64, uint64*) const;

#pragma endregion
	};

	template<>
	void Serializer::deserializeOne(ibytestream*) const;

	template<>
	void Serializer::deserializeOne(const byte*, const uint64, uint64*) const;
}
