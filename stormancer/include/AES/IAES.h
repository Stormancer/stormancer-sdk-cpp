#pragma once

#include "headers.h"
#include "Streams/bytestream.h"

namespace Stormancer
{
	class IAES
	{
	public:

#pragma region public_methods

		// paltform specific factory declaration
		static std::unique_ptr<IAES> createAES(const std::vector<byte>& key);

		virtual ~IAES()
		{
		}

		virtual std::vector<byte> key() const = 0;

		virtual void encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream) = 0;

		virtual void decrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream) = 0;

		virtual void generateRandomIV(byte* ivPtr, std::streamsize ivSize) = 0;

		virtual std::streamsize getBlockSize() = 0;

#pragma endregion
	};
}
