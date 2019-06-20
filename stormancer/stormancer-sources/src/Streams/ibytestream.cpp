#include "stormancer/stdafx.h"
#include "stormancer/Streams/ibytestream.h"

namespace Stormancer
{
	ibytestream::ibytestream()
		: std::basic_istream<byte>(&_buffer)
	{
	}

	ibytestream::ibytestream(byte* data, std::streamsize dataSize)
		: std::basic_istream<byte>(&_buffer)
		, _buffer(data, dataSize)
	{
	}

	ibytestream::ibytestream(ibytestream&& other)
		: std::basic_istream<byte>((std::basic_istream<byte>&&)other)
	{
		auto tmpbuf = rdbuf();
		set_rdbuf(other.rdbuf());
		other.set_rdbuf(tmpbuf);
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

	ibytestream& ibytestream::operator>>(float& value)
	{
		return deserialize(value);
	}

	ibytestream& ibytestream::operator>>(double& value)
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
				bytes.resize(static_cast<std::size_t>(sz));
				read(bytes.data(), sz);
			}
		}
		return (*this);
	}

	std::vector<byte> ibytestream::bytes()
	{
		if (good())
		{
			const byte* first = startPtr();
			const byte* last = first + totalSize() - 1;
			return std::vector<byte>(first, last);
		}
		return std::vector<byte>();
	}

	byte* ibytestream::startPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->startReadPtr();
		}
		return nullptr;
	}

	byte* ibytestream::currentPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->currentReadPtr();
		}
		return nullptr;
	}

	byte* ibytestream::endPtr()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->endReadPtr();
		}
		return nullptr;
	}

	std::streamsize ibytestream::totalSize()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->size();
		}
		return 0;
	}

	std::streamsize ibytestream::availableSize()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
		if (bsb)
		{
			return bsb->in_avail();
		}
		return 0;
	}

	std::streamsize ibytestream::currentPosition()
	{
		auto bsb = static_cast<bytestreambuf*>(rdbuf());
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

	ibytestream& ibytestream::read(char* ptr, std::streamsize size)
	{
		std::basic_istream<byte>::read(reinterpret_cast<byte*>(ptr), size);
		return (*this);
	}
}
