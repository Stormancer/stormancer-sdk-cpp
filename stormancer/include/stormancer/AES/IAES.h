#pragma once

#include "stormancer/headers.h"
#include "stormancer/Streams/bytestream.h"

namespace Stormancer
{
	class IAES
	{
	public:

#pragma region public_methods
	

		virtual ~IAES()
		{
		}

		
		virtual uint16 ivSize();

		virtual void encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream, uint64 keyId) = 0;

		virtual void decrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream, uint64 keyId) = 0;

		virtual void generateRandomIV(byte* ivPtr) = 0;

		virtual std::streamsize getBlockSize() = 0;

#pragma endregion
	};
}
