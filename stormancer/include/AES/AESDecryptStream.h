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

		AESDecryptStream(const std::vector<byte>& key);

		AESDecryptStream(const std::vector<byte>& key, byte* encryptedDataPtr, std::streamsize encryptedDataSize);

		~AESDecryptStream();

		std::vector<byte> bytes() override;

		void decrypt(obytestream* stream);

#pragma endregion

#pragma region private_members

		std::unique_ptr<IAES> _aes;

#pragma endregion
	};
}
