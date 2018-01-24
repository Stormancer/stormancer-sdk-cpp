#pragma once

#include "headers.h"
#include "AES/AES.h"

namespace Stormancer
{
	class AESEncryptStream : public obytestream
	{
	public:

#pragma region public_methods

		AESEncryptStream(const std::vector<byte>& key, bool useIV = true);

		~AESEncryptStream();

		std::vector<byte> bytes() override;

		void encrypt(obytestream* stream);

#pragma endregion

#pragma region private_members

		std::unique_ptr<IAES> _aes;
		bool _useIV = true;

#pragma endregion
	};
}
