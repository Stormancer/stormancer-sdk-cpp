#pragma once

#include <windows.h>
#include <bcrypt.h>
#include "stormancer/headers.h"
#include "stormancer/AES/IAES.h"

namespace Stormancer
{
	class AESWindows : public IAES
	{
	public:

#pragma region public_methods

		AESWindows(const std::vector<byte>& key);

		~AESWindows();

		std::vector<byte> key() const override;

		void encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream) override;

		void decrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream) override;

		void generateRandomIV(byte* ivPtr, std::streamsize ivSize) override;

		virtual std::streamsize getBlockSize() override;

#pragma endregion

#pragma region private_methods

		void initAES();

		void cleanAES();

#pragma endregion

#pragma region private_members

		std::vector<byte> _key;

		BCRYPT_ALG_HANDLE _hAesAlg = NULL;
		BCRYPT_KEY_HANDLE _hKey = NULL;
		PBYTE _pbKeyObject = NULL;
		DWORD _cbBlockLen = 0;

#pragma endregion
	};
}
