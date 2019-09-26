#include "stormancer/stdafx.h"
#include "stormancer/AES/AESEncryptStream.h"

namespace Stormancer
{
	AESEncryptStream::AESEncryptStream(std::shared_ptr<IAES> aes, std::string keyId)
		: _aes(aes)
		, _keyId(keyId)
	{
	}

	AESEncryptStream::~AESEncryptStream()
	{
	}

	std::vector<byte> AESEncryptStream::bytes()
	{
		if (_aes)
		{
			obytestream stream;
			encrypt(stream);
			return stream.bytes();
		}
		return std::vector<byte>();
	}

	void AESEncryptStream::encrypt(obytestream& stream)
	{
		if (_aes)
		{
			byte* dataPtr = startPtr();
			std::streamsize dataSize = currentPosition();
			if (dataPtr && dataSize > 0)
			{
				std::streamsize ivSize = _aes->ivSize();

				auto iv = std::vector<byte>(static_cast<unsigned int>(ivSize));
				_aes->generateRandomIV(iv);
				
				_aes->encrypt(dataPtr, dataSize, iv.data(), ivSize, stream,_keyId);
			}
		}
	}
}
