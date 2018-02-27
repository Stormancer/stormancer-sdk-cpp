#include "stormancer/stdafx.h"
#include "stormancer/AES/AESEncryptStream.h"

namespace Stormancer
{
	AESEncryptStream::AESEncryptStream(const std::vector<byte>& key, bool useIV)
		: _aes(IAES::createAES(key))
		, _useIV(useIV)
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
				std::streamsize ivSize = (_useIV ? _aes->getBlockSize() : 0);
				(*stream) << (uint16)ivSize;

				byte* ivPtr = nullptr;
				if (ivSize > 0)
				{
					ivPtr = new byte[(std::size_t)ivSize];
					_aes->generateRandomIV(ivPtr, ivSize);
					stream->write(ivPtr, ivSize);
				}

				_aes->encrypt(dataPtr, dataSize, ivPtr, ivSize, stream);

				if (ivPtr)
				{
					delete[] ivPtr;
				}
			}
		}
	}
}
