#include "stormancer/stdafx.h"
#include "stormancer/AES/AESDecryptStream.h"

namespace Stormancer
{
	

	AESDecryptStream::AESDecryptStream(std::shared_ptr<IAES> aes, byte* encryptedDataPtr, std::streamsize encryptedDataSize, std::string keyId)
		: obytestream(encryptedDataPtr, encryptedDataSize)
		, _aes(aes)
		, _keyId(keyId)
	{
	}

	AESDecryptStream::~AESDecryptStream()
	{
	}

	std::vector<byte> AESDecryptStream::bytes()
	{
		if (_aes)
		{
			obytestream stream;
			decrypt(stream);
			return stream.bytes();
		}
		return std::vector<byte>();
	}

	void AESDecryptStream::decrypt(obytestream& stream)
	{
		if (_aes)
		{
			byte* dataPtr = startPtr();
			std::streamsize dataSize = rdbuf()->in_avail();
			if (dataPtr != nullptr && dataSize > 0)
			{
				ibytestream ibs(dataPtr, dataSize);

				auto ivSize = _aes->ivSize();

				byte* ivPtr = nullptr;
				if (ivSize > 0)
				{
					ivPtr = new byte[ivSize];
					ibs.read(ivPtr, ivSize);
				}


				std::streamsize dataLeft = ibs.rdbuf()->in_avail();
				byte* encryptedPtr = dataPtr + ivSize;

				_aes->decrypt(encryptedPtr, dataLeft, ivPtr, ivSize, stream,_keyId);


				if (ivPtr)
				{
					delete[] ivPtr;
				}
			}
		}
	}
}
