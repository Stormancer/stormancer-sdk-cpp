#pragma once


#include "stormancer/BuildConfig.h"
#include "stormancer/AES/IAES.h"
#include <memory>
#include <vector>
#include <ios>

namespace Stormancer
{
	class KeyStore;

	class AESLinux : public IAES
	{
	public:

#pragma region public_methods

		AESLinux(std::shared_ptr<KeyStore>);

		~AESLinux();

		std::vector<byte> key() const;

		void encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream& outputStream, std::string keyId) override;

		void decrypt(byte* encryptedDataPtr, std::streamsize encryptedDataSize, byte* ivPtr, std::streamsize ivSize, obytestream& outputStream, std::string keyId) override;

		void generateRandomIV(std::vector<byte> &iv) override;

		virtual std::streamsize getBlockSize() override;

#pragma endregion

	private:
		std::vector<byte> _key;
		static size_t blockSize;

	};
}
