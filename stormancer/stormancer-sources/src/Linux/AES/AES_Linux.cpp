#if defined(LINUX)
#include "stormancer/stdafx.h"
#include "stormancer/Linux/AES/AES_Linux.h"

namespace Stormancer
{
	// platform specific factory implementation
	//std::unique_ptr<IAES> IAES::createAES(const std::vector<byte>& key)
	//{
	//	return std::make_unique<AESNX>(key);
	//}

	AESLinux::AESLinux(std::shared_ptr<KeyStore>)
	{
		

	
	}

	AESLinux::~AESLinux()
	{
		
	}

	std::vector<byte> AESLinux::key() const
	{
		return _key;
	}

	void AESLinux::encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream& outputStream, uint64 keyId)
	{
		throw std::runtime_error("unimplemented");
	}

	void AESLinux::decrypt(byte* encryptedDataPtr, std::streamsize encryptedDataSize, byte* ivPtr, std::streamsize ivSize, obytestream& outputStream, uint64 keyId)
	{
		throw std::runtime_error("unimplemented");
	}

	void AESLinux::generateRandomIV(std::vector<byte> &iv /*ivPtr*/)
	{
		
	}

	std::streamsize AESLinux::getBlockSize()
	{
		return 0;
	}
}
#endif
