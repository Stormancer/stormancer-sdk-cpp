#include "stdafx.h"
#include "Windows/AES/AES_Windows.h"

#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

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
			ss << "Error 0x" << std::hex << status << " returned by BCryptOpenAlgorithmProvider";
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptGetProperty";
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptGetProperty";
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptSetProperty";
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptGenerateSymmetricKey";
			throw std::runtime_error(ss.str());
		}
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

		//
		// Get the output buffer size.
		//
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptEncrypt";
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptEncrypt";
			throw std::runtime_error(ss.str());
		}

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

		//
		// Get the output buffer size.
		//
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
			ss << "Error 0x" << std::hex << status << " returned by BCryptDecrypt";
			throw std::runtime_error(ss.str());
		}

		PBYTE pbDecryptedData = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbDecryptedData);

		if (NULL == pbDecryptedData)
		{
			throw std::runtime_error("memory allocation failed");
		}

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
			ss << "Error 0x" << std::hex << status << " returned by BCryptDecrypt";
			throw std::runtime_error(ss.str());
		}

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
		std::srand((uint32)std::time(0));
		for (int32 i = 0; i < ivSize; i++)
		{
			int32 r = std::rand();
			ivPtr[i] = (byte)(r % 0xFF);
		}
	}

	std::streamsize AESWindows::getBlockSize()
	{
		return _cbBlockLen;
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
