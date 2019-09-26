#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/AES/AES.h"

namespace Stormancer
{
	class AESEncryptStream : public obytestream
	{
	public:

#pragma region public_methods

		AESEncryptStream(std::shared_ptr<IAES> aes, std::string keyId);

		~AESEncryptStream();

		std::vector<byte> bytes() override;

		void encrypt(obytestream& stream);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IAES> _aes;
		std::string _keyId;
#pragma endregion
	};
}
