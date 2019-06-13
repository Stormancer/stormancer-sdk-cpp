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

	obytestream& obytestream::operator<<(const float value)
	{
		return serialize(value);
	}

	obytestream& obytestream::operator<<(const double value)
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
			auto sz = static_cast<std::size_t>(currentPosition());
			std::vector<byte> bytes(sz);
			std::memcpy(bytes.data(), startPtr(), sz);
			return bytes;
		}
		return std::vector<byte>();
	}

	byte* obytestream::startPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->startWritePtr();
		}
		return nullptr;
	}

	byte* obytestream::currentPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentWritePtr();
		}
		return nullptr;
	}

	byte* obytestream::endPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->endWritePtr();
		}
		return nullptr;
	}

	std::streamsize obytestream::totalSize()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->size();
		}
		return 0;
	}

	std::streamsize obytestream::availableSize()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->in_avail();
		}
		return 0;
	}

	std::streamsize obytestream::currentPosition()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentWritePosition();
		}
		return 0;
	}

	void obytestream::dynamic(bool dyn)
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
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
		std::basic_ostream<byte>::write(reinterpret_cast<const byte*>(ptr), size);
		return (*this);
	}
}
