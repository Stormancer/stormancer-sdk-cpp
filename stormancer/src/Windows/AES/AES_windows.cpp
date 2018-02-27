#include "stormancer/stdafx.h"
#include "stormancer/Windows/AES/AES_Windows.h"

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
	// paltform specific factory implementation
	std::unique_ptr<IAES> IAES::createAES(const std::vector<byte>& key)
	{
		return std::make_unique<AESWindows>(key);
	}

	AESWindows::AESWindows(const std::vector<byte>& key)
		: _key(key)
	{
		initAES();
	}

	AESWindows::~AESWindows()
	{
		cleanAES();
	}

	std::vector<byte> AESWindows::key() const
	{
		return _key;
	}

	void AESWindows::encrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream)
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		if (ivPtr == nullptr || ivSize == 0 || ivSize != _cbBlockLen)
		{
			ivSize = 0;
			ivPtr = nullptr;
		}

		DWORD cbCipherText = 0;

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptEncrypt(
			_hKey,
			dataPtr,
			(ULONG)dataSize,
			NULL,
			ivPtr,
			(ULONG)ivSize,
			NULL,
			0,
			&cbCipherText,
			BCRYPT_BLOCK_PADDING)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptEncrypt";
			throw std::runtime_error(ss.str());
		}

		PBYTE pbCipherText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbCipherText);
		if (NULL == pbCipherText)
		{
			throw std::runtime_error("memory allocation failed");
		}

		DWORD cbData = 0;

		// Use the key to encrypt the plaintext buffer.
		// For block sized messages, block padding will add an extra block.
		if (!NT_SUCCESS(status = BCryptEncrypt(
			_hKey,
			dataPtr,
			(ULONG)dataSize,
			NULL,
			ivPtr,
			(ULONG)ivSize,
			pbCipherText,
			cbCipherText,
			&cbData,
			BCRYPT_BLOCK_PADDING)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptEncrypt";
			throw std::runtime_error(ss.str());
		}

		// Write the encrypted data in the output stream
		if (outputStream)
		{
			uint32 encryptedSize = cbCipherText;
			(*outputStream) << encryptedSize;

			outputStream->write(pbCipherText, cbCipherText);
		}

		if (pbCipherText)
		{
			HeapFree(GetProcessHeap(), 0, pbCipherText);
		}
	}

	void AESWindows::decrypt(byte* dataPtr, std::streamsize dataSize, byte* ivPtr, std::streamsize ivSize, obytestream* outputStream)
	{
		if (ivSize != _cbBlockLen)
		{
			throw std::runtime_error("Id size does not match with the block size");
		}

		NTSTATUS status = STATUS_UNSUCCESSFUL;

		DWORD cbDecryptedData = 0;

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptDecrypt(
			_hKey,
			dataPtr,
			(ULONG)dataSize,
			NULL,
			ivPtr,
			(ULONG)ivSize,
			NULL,
			0,
			&cbDecryptedData,
			BCRYPT_BLOCK_PADDING)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptDecrypt";
			throw std::runtime_error(ss.str());
		}

		PBYTE pbDecryptedData = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbDecryptedData);

		if (NULL == pbDecryptedData)
		{
			throw std::runtime_error("memory allocation failed");
		}

		// Decrypt the data
		if (!NT_SUCCESS(status = BCryptDecrypt(
			_hKey,
			dataPtr,
			(ULONG)dataSize,
			NULL,
			ivPtr,
			(ULONG)ivSize,
			pbDecryptedData,
			cbDecryptedData,
			&cbDecryptedData,
			BCRYPT_BLOCK_PADDING)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptDecrypt";
			throw std::runtime_error(ss.str());
		}

		// Write the decrypted data in the output stream
		if (outputStream)
		{
			outputStream->write(pbDecryptedData, cbDecryptedData);
		}

		if (pbDecryptedData)
		{
			HeapFree(GetProcessHeap(), 0, pbDecryptedData);
		}
	}

	void AESWindows::generateRandomIV(byte* ivPtr, std::streamsize ivSize)
	{
		for (int32 i = 0; i < ivSize; i++)
		{
			//int32 r = std::rand();
			//ivPtr[i] = (byte)r;
			ivPtr[i] = (byte)i;
		}
	}

	std::streamsize AESWindows::getBlockSize()
	{
		return _cbBlockLen;
	}

	void AESWindows::initAES()
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		// Open an algorithm handle.
		if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
			&_hAesAlg,
			BCRYPT_AES_ALGORITHM,
			NULL,
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptOpenAlgorithmProvider";
			throw std::runtime_error(ss.str());
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
			throw std::runtime_error(ss.str());
		}

		// Allocate the key object on the heap.
		_pbKeyObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
		if (NULL == _pbKeyObject)
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
			throw std::runtime_error(ss.str());
		}

		if (!NT_SUCCESS(status = BCryptSetProperty(
			_hAesAlg,
			BCRYPT_CHAINING_MODE,
			(PBYTE)BCRYPT_CHAIN_MODE_CBC,
			sizeof(BCRYPT_CHAIN_MODE_CBC),
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptSetProperty";
			throw std::runtime_error(ss.str());
		}

		// Generate the key from supplied input key bytes.
		if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(
			_hAesAlg,
			&_hKey,
			_pbKeyObject,
			cbKeyObject,
			(PBYTE)_key.data(),
			(ULONG)_key.size(),
			0)))
		{
			std::stringstream ss;
			ss << "Error " << getErrorString(status) << " returned by BCryptGenerateSymmetricKey";
			throw std::runtime_error(ss.str());
		}
	}

	void AESWindows::cleanAES()
	{
		if (_hAesAlg)
		{
			BCryptCloseAlgorithmProvider(_hAesAlg, 0);
		}

		if (_hKey)
		{
			BCryptDestroyKey(_hKey);
		}

		if (_pbKeyObject)
		{
			HeapFree(GetProcessHeap(), 0, _pbKeyObject);
		}
	}
}
