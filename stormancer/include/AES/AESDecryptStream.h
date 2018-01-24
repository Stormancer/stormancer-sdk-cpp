#pragma once

#include "headers.h"
#include "Streams/bytestream.h"
#include "AES/AES.h"

namespace Stormancer
{
	class AESDecryptStream : public obytestream
	{
	public:

#pragma region public_methods

		AESDecryptStream(const std::vector<byte>& key, byte* dataPtr = nullptr, std::streamsize dataSize = 0);

		~AESDecryptStream();

		std::vector<byte> bytes() override;

		void decrypt(obytestream* stream);

#pragma endregion

#pragma region private_members

		std::unique_ptr<IAES> _aes;

#pragma endregion
	};
}
