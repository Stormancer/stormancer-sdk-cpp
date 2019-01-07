#include "stormancer/stdafx.h"
#include "stormancer/Windows/AES/AES_Windows.h"
#include "stormancer/KeyStore.h"
#include <sstream>
#include <stdexcept>
#include <string>

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#endif

#ifndef STATUS_AUTH_TAG_MISMATCH
#define STATUS_AUTH_TAG_MISMATCH         ((NTSTATUS)0xC000A002L)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif

#ifndef STATUS_INVALID_BUFFER_SIZE
#define STATUS_INVALID_BUFFER_SIZE       ((NTSTATUS)0xC0000206L)
#endif

#ifndef STATUS_INVALID_HANDLE
#define STATUS_INVALID_HANDLE            ((NTSTATUS)0xC0000008L)    // winnt
#endif

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)    // winnt
#endif

#ifndef STATUS_NOT_SUPPORTED
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#endif

#ifndef STATUS_DATA_ERROR
#define STATUS_DATA_ERROR                ((NTSTATUS)0xC000003E)
#endif

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

std::string getErrorString(NTSTATUS err)
{
	std::stringstream ss;
	switch (err)
	{
	case STATUS_SUCCESS:
		ss << "STATUS_SUCCESS";
		break;
	case STATUS_AUTH_TAG_MISMATCH:
		ss << "STATUS_AUTH_TAG_MISMATCH";
		break;
	case STATUS_BUFFER_TOO_SMALL:
		ss << "STATUS_BUFFER_TOO_SMALL";
		break;
	case STATUS_INVALID_BUFFER_SIZE:
		ss << "STATUS_INVALID_BUFFER_SIZE";
		break;
	case STATUS_INVALID_HANDLE:
		ss << "STATUS_INVALID_HANDLE";
		break;
	case STATUS_INVALID_PARAMETER:
		ss << "STATUS_INVALID_PARAMETER";
		break;
	case STATUS_NOT_SUPPORTED:
		ss << "STATUS_NOT_SUPPORTED";
		break;
	case STATUS_DATA_ERROR:
		ss << "STATUS_DATA_ERROR";
		break;
	default:
		ss << "0x" << std::hex << err;
		return ss.str();
		break;
	}
	ss << " (0x" << std::hex << err << ")";
	return ss.str();
}

namespace Stormancer
{


	AESWindows::AESWindows(std::shared_ptr<KeyStore> keyStore)
		: _key(keyStore)
	{
		
	}

	AESWindows::~AESWindows()
	{
		cleanAES();
	}



	void AESWindows::encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream, uint64 keyId)
	{
		initAES(keyId);
		
		NTSTATUS status = STATUS_UNSUCCESSFUL;





		BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
		BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
		std::vector<BYTE> authTag(authTagLengths.dwMinLength);
		std::vector<BYTE> macContext(authTagLengths.dwMaxLength);
		ULONG tagSize = 12;
		std::vector<BYTE> tag(tagSize);

		authInfo.pbNonce = ivPtr;
		authInfo.cbNonce = (ULONG)ivSize;
		authInfo.pbTag = tag.data();
		authInfo.cbTag = tagSize;
		authInfo.pbMacContext = &macContext[0];
		authInfo.cbMacContext = (ULONG)macContext.size();

		// IV value is ignored on first call to BCryptDecrypt.
		// This buffer will be used to keep internal IV used for chaining.
		std::vector<BYTE> contextIV(_cbBlockLen);
		std::vector<BYTE> encrypted(((ULONG)dataSize));

		DWORD bytesProcessed;
		// Use the key to encrypt the plaintext buffer.
		// For block sized messages, block padding will add an extra block.
		if (!NT_SUCCESS(status = BCryptEncrypt(
			_keyHandles[keyId],
			dataPtr,
			(ULONG)dataSize,
			&authInfo,
			&contextIV[0], (ULONG)contextIV.size(),
			&encrypted[0], (ULONG)encrypted.size(),
			&bytesProcessed,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptEncrypt";
			throw std::runtime_error(ss.str().c_str());
		}

		// Write the encrypted data in the output stream
		if (outputStream)
		{



			outputStream->write(ivPtr, ivSize);
			outputStream->write(encrypted.data(), encrypted.size());
			outputStream->write(tag.data(), tag.size());
		}
	}

	void AESWindows::decrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream, uint64 keyId)
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;


		BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
		BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
		std::vector<BYTE> authTag(authTagLengths.dwMinLength);
		std::vector<BYTE> macContext(authTagLengths.dwMaxLength);
		ULONG tagSize = 12;

		authInfo.pbNonce = ivPtr;
		authInfo.cbNonce = (ULONG)ivSize;
		authInfo.pbTag = dataPtr + dataSize - tagSize;
		authInfo.cbTag = tagSize;
		authInfo.pbMacContext = &macContext[0];
		authInfo.cbMacContext = (ULONG)macContext.size();

		// IV value is ignored on first call to BCryptDecrypt.
		// This buffer will be used to keep internal IV used for chaining.
		std::vector<BYTE> contextIV(_cbBlockLen);
		std::vector<BYTE> decrypted(((ULONG)dataSize) - tagSize);

		DWORD bytesDone;
		// Decrypt the data
		if (!NT_SUCCESS(status = BCryptDecrypt(
			_keyHandles[keyId],
			dataPtr, (ULONG)dataSize - tagSize,
			&authInfo,
			&contextIV[0], (ULONG)contextIV.size(),
			&decrypted[0], (ULONG)decrypted.size(),
			&bytesDone,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptDecrypt";
			throw std::runtime_error(ss.str().c_str());
		}

		// Write the decrypted data in the output stream
		if (outputStream)
		{
			outputStream->write(&decrypted[0], bytesDone);
		}


	}

	void AESWindows::generateRandomIV(std::vector<BYTE>& iv)
	{
		for (std::size_t i = 0; i < iv.size(); i++)
		{
			int32 r = std::rand();
			iv.data()[i] = (byte)r;
		}
	}

	std::streamsize AESWindows::getBlockSize()
	{
		return _cbBlockLen;
	}

	bool AESWindows::initAES(uint64 keyId)
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		if (_keyHandles.find(keyId) != _keyHandles.end())
		{
			return true;// already initialized
		}
		PBYTE key = _key->getKey(keyId).data();


		// Open an algorithm handle.
		if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
			&_hAesAlg,
			BCRYPT_AES_ALGORITHM,
			NULL,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptOpenAlgorithmProvider";
			throw std::runtime_error(ss.str().c_str());
		}
		if (!NT_SUCCESS(status = BCryptSetProperty(
			_hAesAlg,
			BCRYPT_CHAINING_MODE,
			(PBYTE)BCRYPT_CHAIN_MODE_GCM,
			sizeof(BCRYPT_CHAIN_MODE_GCM),
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptSetProperty";
			throw std::runtime_error(ss.str().c_str());
		}

		DWORD cbKeyObject = 0;
		DWORD cbData = 0;

		// Calculate the size of the buffer to hold the KeyObject.
		if (!NT_SUCCESS(status = BCryptGetProperty(
			_hAesAlg,
			BCRYPT_OBJECT_LENGTH,
			(PBYTE)&cbKeyObject,
			sizeof(DWORD),
			&cbData,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptGetProperty";
			throw std::runtime_error(ss.str().c_str());
		}

		// Allocate the key object on the heap.
		_keyPointers[keyId] = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
		if (NULL == _keyPointers[keyId])
		{
			throw std::runtime_error("memory allocation failed");
		}

		// Calculate the block length for the IV.
		if (!NT_SUCCESS(status = BCryptGetProperty(
			_hAesAlg,
			BCRYPT_BLOCK_LENGTH,
			(PBYTE)&_cbBlockLen,
			sizeof(DWORD),
			&cbData,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptGetProperty";
			throw std::runtime_error(ss.str().c_str());
		}
		DWORD bytesDone = 0;

		if (!NT_SUCCESS(status = BCryptGetProperty(_hAesAlg, BCRYPT_AUTH_TAG_LENGTH, (BYTE*)&authTagLengths, sizeof(authTagLengths), &bytesDone, 0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned byBCryptGetProperty(BCRYPT_AUTH_TAG_LENGTH)";
			throw std::runtime_error(ss.str().c_str());
		}


		// Generate the key from supplied input key bytes.
		if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(
			_hAesAlg,
			&_keyHandles[keyId],
			_keyPointers[keyId],
			cbKeyObject,
			key,
			(ULONG)32,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptGenerateSymmetricKey";
			throw std::runtime_error(ss.str().c_str());
		}
		return true;

	}

	void AESWindows::cleanAES()
	{
		if (_hAesAlg)
		{
			BCryptCloseAlgorithmProvider(_hAesAlg, 0);
		}
		for (auto key : this->_keyHandles)
		{
			BCryptDestroyKey(key.second);
		}
		_keyHandles.clear();

		for (auto key : _keyPointers)
		{
			HeapFree(GetProcessHeap(), 0, key.second);
		}
		_keyPointers.clear();
	}
}
