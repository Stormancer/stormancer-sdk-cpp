#include "stormancer/stdafx.h"
#include "stormancer/AES/AESEncryptStream.h"

namespace Stormancer
{
	AESEncryptStream::AESEncryptStream(std::shared_ptr<IAES> aes, uint64 keyId)
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
			encrypt(&stream);
			return stream.bytes();
		}
		return std::vector<byte>();
	}

	void AESEncryptStream::encrypt(obytestream* stream)
	{
		if (_aes)
		{
			byte* dataPtr = startPtr();
			std::streamsize dataSize = writtenBytesCount();
			if (dataPtr && dataSize > 0)
			{
				std::streamsize ivSize = _aes->ivSize();

				byte* ivPtr = nullptr;

				ivPtr = new byte[(std::size_t)ivSize];
				_aes->generateRandomIV(ivPtr);
				stream->write(ivPtr, ivSize);


				_aes->encrypt(dataPtr, dataSize, ivPtr, ivSize, stream,_keyId);

				if (ivPtr)
				{
					delete[] ivPtr;
				}
			}
		}
	}
}
