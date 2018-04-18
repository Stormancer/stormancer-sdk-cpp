#include "stormancer/stdafx.h"
#include "stormancer/Helpers.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/AES/AESPacketTransform.h"
#include "stormancer/AES/AESEncryptStream.h"
#include "stormancer/AES/AESDecryptStream.h"

namespace Stormancer
{
	AESPacketTransform::AESPacketTransform()
	{
	}

	void AESPacketTransform::onSend(Writer& writer, const TransformMetadata& transformMetadata)
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

					std::vector<byte> key = {
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
						0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
						0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
					};

					AESEncryptStream aesStream(key, true);
					if (writerCopy)
					{
						writerCopy(&aesStream);
					}
					aesStream.encrypt(stream);
				};
			}
		}
	}

	void AESPacketTransform::onReceive(ibytestream* stream, const TransformMetadata& /*transformMetadata*/)
	{
		auto bc = stream->rdbuf()->sgetc();
		if (bc != bytestreambuf::traits_type::eof())
		{
			byte b = bytestreambuf::traits_type::to_char_type(bc);
			if (b == (byte)MessageIDTypes::ID_ENCRYPTED)
			{
				stream->rdbuf()->sbumpc();

				std::vector<byte> key = {
					0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
					0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
					0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
					0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
				};

				byte* dataPtr = stream->startPtr() + 1;
				std::streamsize dataSize = stream->rdbuf()->in_avail();

				AESDecryptStream aesStream(key, dataPtr, dataSize);
				obytestream os;
				aesStream.decrypt(&os);

				byte* decryptedPtr = os.startPtr();
				std::streamsize decryptedSize = os.writtenBytesCount();

				std::memcpy(dataPtr, decryptedPtr, (std::size_t)decryptedSize);
				stream->rdbuf()->pubsetbuf(dataPtr, decryptedSize);
			}
		}
	}
}
