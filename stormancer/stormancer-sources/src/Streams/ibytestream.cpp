#include "stormancer/stdafx.h"
#include "stormancer/Streams/ibytestream.h"

namespace Stormancer
{
	ibytestream::ibytestream()
		: std::basic_istream<byte>(&_buffer)
	{
	}

	ibytestream::ibytestream(byte* data, std::streamsize sz)
		: std::basic_istream<byte>(&_buffer)
		, _buffer(data, sz)
	{
	}

	ibytestream& ibytestream::operator>>(char& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(int8& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(uint8& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(int16& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(uint16& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(int32& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(uint32& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(int64& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(uint64& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(float32& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(float64& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(bool& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(void*& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(std::vector<byte>& bytes)
	{
		auto buf = rdbuf();
		if (buf)
		{
			std::streamsize sz = buf->in_avail();
			if (sz > 0)
			{
				bytes.resize((std::size_t)sz);
				read(bytes.data(), sz);
			}
		}
		return (*this);
	}

	std::vector<byte> ibytestream::bytes()
	{
		std::vector<byte> bytes;
		if (good())
		{
			int sz = (int)totalSize();
			bytes.resize(sz);
			std::memcpy(bytes.data(), startPtr(), sz);
		}
		return bytes;
	}

	byte* ibytestream::startPtr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->startReadPtr();
		}
		return nullptr;
	}

	byte* ibytestream::currentPtr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentReadPtr();
		}
		return nullptr;
	}

	byte* ibytestream::endPtr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->endReadPtr();
		}
		return nullptr;
	}

	std::streamsize ibytestream::totalSize()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->size();
		}
		return 0;
	}

	std::streamsize ibytestream::availableSize()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->in_avail();
		}
		return 0;
	}

	std::streamsize ibytestream::currentPosition()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentReadPosition();
		}
		return 0;
	}

	ibytestream& ibytestream::read(byte* ptr, std::streamsize size)
	{
		std::basic_istream<byte>::read(ptr, size);
		return (*this);
	}
#if !defined(_LIBCPP_VERSION)
	ibytestream& ibytestream::read(char* ptr, std::streamsize size)
	{
		std::basic_istream<byte>::read((byte*)ptr, size);
		return (*this);
	}
#endif
}
