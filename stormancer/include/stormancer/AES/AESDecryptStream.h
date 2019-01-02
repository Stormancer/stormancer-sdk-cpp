#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Streams/bytestream.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/AES/AES.h"
#include <memory>
#include <vector>

namespace Stormancer
{
	class AESDecryptStream : public obytestream
	{
	public:

#pragma region public_methods

		
		AESDecryptStream(std::shared_ptr<IAES> aes, byte* encryptedDataPtr, std::streamsize encryptedDataSize, uint64 keyId);

		~AESDecryptStream();

		std::vector<byte> bytes() override;

		void decrypt(obytestream* stream);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IAES> _aes;
		uint64 _keyId;
#pragma endregion
	};
}
