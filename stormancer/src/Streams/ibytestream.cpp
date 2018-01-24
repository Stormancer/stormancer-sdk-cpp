#include "stdafx.h"
#include "Streams/ibytestream.h"

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
			int sz = (int)size();
			bytes.resize(sz);
			std::memcpy(bytes.data(), ptr(), sz);
		}
		return bytes;
	}

	byte* ibytestream::ptr()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->ptr();
		}
		return nullptr;
	}

	int ibytestream::size()
	{
		auto bsb = dynamic_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->gcount();
		}
		return 0;
	}

	ibytestream& ibytestream::read(byte* ptr, std::streamsize size)
	{
		std::basic_istream<byte>::read(ptr, size);
		return (*this);
	}

	ibytestream& ibytestream::read(char* ptr, std::streamsize size)
	{
		std::basic_istream<byte>::read((byte*)ptr, size);
		return (*this);
	}

}
