#include "stormancer/stdafx.h"
#include "stormancer/Helpers.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/AES/AESPacketTransform.h"
#include "stormancer/AES/AESEncryptStream.h"
#include "stormancer/AES/AESDecryptStream.h"
#include "stormancer/AES/IAES.h"

namespace Stormancer
{
	AESPacketTransform::AESPacketTransform(std::shared_ptr<IAES> aes)
		:_aes(aes)
	{
	}

	void AESPacketTransform::onSend(Writer& writer, uint64 peerId, const TransformMetadata& transformMetadata)
	{
		std::string str("crypt");
		if (mapContains(transformMetadata.sceneMetadata, str))
		{
			auto& cryptedRoutes = transformMetadata.sceneMetadata.at(str);
			if (cryptedRoutes.find(";ALL;") != std::string::npos || cryptedRoutes.find(";+" + transformMetadata.routeName + ";") != std::string::npos)
			{
				auto writerCopy = writer;
				writer = [=](obytestream* stream) {
					(*stream) << (byte)MessageIDTypes::ID_ENCRYPTED;



					AESEncryptStream aesStream(_aes, peerId);
					if (writerCopy)
					{
						writerCopy(&aesStream);
					}
					aesStream.encrypt(stream);
				};
			}
		}
	}

	void AESPacketTransform::onReceive(ibytestream* stream, uint64 peerId)
	{
		auto bc = stream->rdbuf()->sgetc();
		if (bc != bytestreambuf::traits_type::eof())
		{

			//stream->rdbuf()->sbumpc();



			byte* dataPtr = stream->startPtr()+1;
			std::streamsize dataSize = stream->rdbuf()->in_avail();

			AESDecryptStream aesStream(_aes, dataPtr, dataSize, peerId);
			obytestream os;
			aesStream.decrypt(&os);

			byte* decryptedPtr = os.startPtr();
			std::streamsize decryptedSize = os.writtenBytesCount();

			std::memcpy(dataPtr, decryptedPtr, (std::size_t)decryptedSize);
			stream->rdbuf()->pubsetbuf(dataPtr, decryptedSize);

		}
	}
}
