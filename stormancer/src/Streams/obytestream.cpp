#include "stormancer/stdafx.h"
#include "stormancer/Streams/obytestream.h"

namespace Stormancer
{
	obytestream::obytestream()
		: std::basic_ostream<byte>(&_buffer)
	{
	}

	obytestream::obytestream(byte* dataPtr, std::streamsize dataSize, bool allowExtend)
		: std::basic_ostream<byte>(&_buffer)
		, _buffer(dataPtr, dataSize, allowExtend)
	{
	}

	obytestream& obytestream::operator<<(const char value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const int8 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const uint8 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const int16 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const uint16 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const int32 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const uint32 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const int64 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const uint64 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const float32 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const float64 value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const bool value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const void* value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const std::vector<byte>& bytes)
	{
		if (good())
		{
			write(bytes.data(), bytes.size());
		}
		return (*this);
	}

	std::vector<byte> obytestream::bytes()
	{
		if (good())
		{
			auto sz = static_cast<std::size_t>(writtenBytesCount());
			std::vector<byte> bytes(sz);
			std::memcpy(bytes.data(), startPtr(), sz);
			return bytes;
		}
		return std::vector<byte>();
	}

	byte* obytestream::startPtr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->startWritePtr();
		}
		return nullptr;
	}

	byte* obytestream::currentPtr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentWritePtr();
		}
		return nullptr;
	}

	std::streamsize obytestream::size()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->size();
		}
		return 0;
	}

	std::streamsize obytestream::writtenBytesCount()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->writtenBytesCount();
		}
		return 0;
	}

	void obytestream::dynamic(bool dyn)
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			bsb->dynamic(dyn);
		}
	}

	obytestream& obytestream::write(const byte* ptr, std::streamsize size)
	{
		std::basic_ostream<byte>::write(ptr, size);
		return (*this);
	}

	obytestream& obytestream::write(const char* ptr, std::streamsize size)
	{
		std::basic_ostream<byte>::write((byte*)ptr, size);
		return (*this);
	}
}
