#include "stormancer/stdafx.h"
#include "stormancer/Streams/bytestreambuf.h"
#include <algorithm>

namespace Stormancer
{
	bytestreambuf::bytestreambuf()
	{
		init();
	}

	bytestreambuf::bytestreambuf(byte* dataPtr, std::streamsize dataSize, bool isDynamic)
	{
		init();
		if (dataPtr == nullptr || dataSize == 0)
		{
			dataPtr = nullptr;
			dataSize = 0;
		}
		if (dataPtr != nullptr && dataSize > 0)
		{
			setbuf(dataPtr, dataSize);
		}
		dynamic(isDynamic);
	}

	bytestreambuf::~bytestreambuf()
	{
		tidy();
	}

	byte* bytestreambuf::startReadPtr()
	{
		return eback();
	}

	byte* bytestreambuf::currentReadPtr()
	{
		return gptr();
	}

	byte* bytestreambuf::endReadPtr()
	{
		return egptr();
	}

	byte* bytestreambuf::startWritePtr()
	{
		return pbase();
	}

	byte* bytestreambuf::currentWritePtr()
	{
		return pptr();
	}

	byte* bytestreambuf::endWritePtr()
	{
		return epptr();
	}

	std::streamsize bytestreambuf::size()
	{
		if (_mode & Write)
		{
			byte* first = eback();
			byte* last = egptr();
			if (first && last)
			{
				return (last - first);
			}
		}
		else if (_mode & Read)
		{
			byte* first = pbase();
			byte* last = epptr();
			if (first && last)
			{
				return (last - first);
			}
		}
		return 0;
	}

	std::streamsize bytestreambuf::currentReadPosition()
	{
		byte* first = eback();
		byte* next = gptr();
		if (first && next)
		{
			return (next - first);
		}
		return 0;
	}

	std::streamsize bytestreambuf::currentWritePosition()
	{
		byte* first = pbase();
		byte* next = pptr();
		if (first && next)
		{
			return (next - first);
		}
		return 0;
	}

	void bytestreambuf::dynamic(bool dyn)
	{
		if (dyn)
		{
			// add dynamic flag
			_mode |= Dynamic;
		}
		else
		{
			// remove dynamic flag
			_mode &= (~Dynamic);
		}
	}

	std::basic_streambuf<byte>* bytestreambuf::setbuf(byte* s, std::streamsize size)
	{
		tidy();
		setg(s, s, (s + size));
		setp(s, (s + size));
		_mode = (Read | Write);
		return (this);
	}

	bytestreambuf::pos_type bytestreambuf::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
	{
		if ((which & std::ios_base::in) && (which & std::ios_base::out) && (way != std::ios_base::beg) && (way != std::ios_base::end))
		{
			return pos_type(off_type(-1));
		}

		off_type newoff = 0;

		if ((_mode & Read) && (which & std::ios_base::in))
		{
			byte* first = eback();
			byte* next = gptr();
			byte* last = egptr();

			if (next == nullptr)
			{
				return pos_type(off_type(-1));
			}

			if (way == std::ios_base::beg)
			{
				if (first == nullptr)
				{
					return pos_type(off_type(-1));
				}

				newoff = 0;
			}
			else if (way == std::ios_base::cur)
			{
				newoff = next - first;
			}
			else if (way == std::ios_base::end)
			{
				if (last == nullptr)
				{
					return pos_type(off_type(-1));
				}

				newoff = last - first;
			}

			off_type finaloff = newoff + off;

			if (finaloff < 0 || finaloff >(last - first))
			{
				return pos_type(off_type(-1));
			}

			next = first + finaloff;

			if (!setgIfValid(first, next, last))
			{
				return pos_type(off_type(-1));
			}
		}

		if ((_mode & Write) && (which & std::ios_base::out))
		{
			byte* first = pbase();
			byte* next = pptr();
			byte* last = epptr();

			if (next == nullptr)
			{
				return pos_type(off_type(-1));
			}

			if (way == std::ios_base::beg)
			{
				if (first == nullptr)
				{
					return pos_type(off_type(-1));
				}

				newoff = 0;
			}
			else if (way == std::ios_base::cur)
			{
				newoff = next - first;
			}
			else if (way == std::ios_base::end)
			{
				if (last == nullptr)
				{
					return pos_type(off_type(-1));
				}

				newoff = last - first;
			}

			off_type finaloff = newoff + off;

			if (finaloff < 0 || finaloff >(last - first))
			{
				return pos_type(off_type(-1));
			}

			next = first + finaloff;

			if (!setpIfValid(first, next, last))
			{
				return pos_type(off_type(-1));
			}
		}

		return pos_type(newoff);
	}

	bytestreambuf::pos_type bytestreambuf::seekpos(pos_type pos, std::ios_base::openmode which)
	{
		if ((_mode & Read) && (which & std::ios_base::in))
		{
			if (gptr() == nullptr || pos < 0)
			{
				return pos_type(off_type(-1));
			}

			byte* first = eback();
			byte* next = first + pos;
			byte* last = egptr();

			if (!setgIfValid(first, next, last))
			{
				return pos_type(off_type(-1));
			}
		}

		if ((_mode & Write) && (which & std::ios_base::out))
		{
			if (pptr() == nullptr || pos < 0)
			{
				return pos_type(off_type(-1));
			}

			byte* first = pbase();
			byte* next = first + pos;
			byte* last = epptr();

			if (!setpIfValid(first, next, last))
			{
				return pos_type(off_type(-1));
			}
		}

		return pos;
	}

	std::streamsize bytestreambuf::showmanyc()
	{
		if (_mode & Read)
		{
			byte* next = gptr();
			byte* last = egptr();
			if (next != last)
			{
				return last - next;
			}
			else if (_mode & Write)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	std::streamsize bytestreambuf::xsgetn(byte* s, std::streamsize n)
	{
		std::streamsize res = 0;

		if (s)
		{
			if (_mode & Read)
			{
				std::streamsize size = std::min(showmanyc(), n);
				byte* first = eback();
				byte* next = gptr();
				byte* last = egptr();
				std::memcpy(s, next, static_cast<std::size_t>(size));
				next += size;
				setg(first, next, last);
				res += size;
			}
		}

		return res;
	}

	bytestreambuf::int_type bytestreambuf::underflow()
	{
		if (_mode & Read)
		{
			byte* next = gptr();
			byte* last = egptr();
			if (next != last)
			{
				return traits_type::to_int_type(gptr()[0]);
			}
		}

		return traits_type::eof();
	}

	bytestreambuf::int_type bytestreambuf::uflow()
	{
		if (_mode & Read)
		{
			byte* next = gptr();
			byte* last = egptr();
			if (next != last)
			{
				byte* first = eback();
				int_type ch = traits_type::to_int_type(gptr()[0]);
				next += 1;
				setg(first, next, last);
				return ch;
			}
		}

		return traits_type::eof();
	}

	bytestreambuf::int_type bytestreambuf::pbackfail(int_type c)
	{
		if (_mode & Read)
		{
			byte* first = eback();
			byte* next = gptr();
			if (next > first)
			{
				byte* last = egptr();
				next--;
				if (_mode & Write)
				{
					next[0] = (byte)c;
				}
				else if (!traits_type::eq(next[0], (byte)c))
				{
					return traits_type::eof();
				}
				setg(first, next, last);
				return traits_type::not_eof(c);
			}
		}

		return traits_type::eof();
	}

	std::streamsize bytestreambuf::xsputn(const char_type* s, std::streamsize n)
	{
		std::streamsize wc = 0;

		// Check data to write
		if (s && n > 0)
		{
			// Check write mode
			if (_mode & Write)
			{
				byte* first = pbase();
				byte* next = pptr();
				byte* last = epptr();
				std::streamsize avail = 0;
				if (first && next && last)
				{
					avail = last - next;
				}
				// Reserve more memory if necessary
				if (avail < n)
				{
					if (_mode & Dynamic)
					{
						std::streamsize targetSize = n;
						if (next && first)
						{
							targetSize += (next - first);
						}
						grow(targetSize);
						first = pbase();
						next = pptr();
						last = epptr();
						avail = last - next;
					}
				}
				// Write data if the is enough available memory
				std::streamsize sz = std::min(avail, n);
				if (sz > 0)
				{
					std::memcpy(next, s, static_cast<std::size_t>(sz));
					next += sz;
					setp(first, last);
					pbump((int)(next - first));
					updateReadAfterWrite();
					wc += sz;
				}
			}
		}

		return wc;
	}

	bytestreambuf::int_type bytestreambuf::overflow(int_type c)
	{
		if (_mode & Write)
		{
			byte* next = pptr();
			byte* last = epptr();
			if (next && (next != last))
			{
				next[0] = (byte)c;
			}
			updateReadAfterWrite();
		}

		return traits_type::eof();
	}

	void bytestreambuf::tidy()
	{
		if (_mode & Allocated)
		{
			byte* ptr = pbase();

			if (!ptr)
			{
				ptr = eback();
			}

			if (ptr)
			{
				delete[] ptr;
			}
		}
		reset();
	}

	void bytestreambuf::reset()
	{
		setg(nullptr, nullptr, nullptr);
		setp(nullptr, nullptr);
		_mode = 0;
	}

	void bytestreambuf::init(std::streamsize size)
	{
		byte* ptr;
		byte* ptrWriteLast;
		if (size > 0)
		{
			ptr = new byte[(unsigned int)size]();
			ptrWriteLast = ptr + size;
		}
		else
		{
			ptr = nullptr;
			ptrWriteLast = nullptr;
		}
		setg(ptr, ptr, ptr);
		setp(ptr, ptrWriteLast);
		_mode = (Read | Write | Dynamic | Allocated);
	}

	void bytestreambuf::grow(std::streamsize minSize)
	{
		if ((minSize > 0) && (_mode & Dynamic))
		{
			Mode oldMode = _mode;

			byte* oldReadFirst = eback();
			byte* oldReadNext = gptr();
			byte* oldReadLast = egptr();

			byte* oldWriteFirst = pbase();
			byte* oldWriteNext = pptr();
			byte* oldWriteLast = epptr();

			std::streamsize oldSize = 0;
			if (oldWriteFirst && oldWriteLast)
			{
				oldSize = oldWriteLast - oldWriteFirst;
			}
			std::streamsize newSize = (oldSize > 0 ? oldSize : 32);
			while (newSize < minSize)
			{
				newSize *= 2;
			}

			reset();
			init(newSize);

			_mode = oldMode;

			if (oldSize > 0)
			{
				byte* writeFirst = pbase();
				byte* writeNext = pptr();
				byte* writeLast = epptr();

				std::memcpy(writeFirst, oldWriteFirst, static_cast<std::size_t>(oldSize));

				writeNext = writeFirst + (oldWriteNext - oldWriteFirst);

				setp(writeFirst, writeLast);
				pbump((int)(oldWriteNext - oldWriteFirst));

				byte* readFirst = (oldReadFirst ? writeFirst : nullptr);
				byte* readNext = (oldReadNext ? writeFirst + (oldReadNext - oldReadFirst) : nullptr);
				byte* readLast = (oldReadLast ? writeLast : nullptr);

				setg(readFirst, readNext, readLast);
			}

			if ((oldMode & Allocated) && oldWriteFirst)
			{
				delete[] oldWriteFirst;
			}
		}
	}

	inline bool bytestreambuf::setgIfValid(byte* first, byte* next, byte* last)
	{
		if ((next >= first) && (next <= last))
		{
			setg(first, next, last);
			return true;
		}
		return false;
	}

	inline bool bytestreambuf::setpIfValid(byte* first, byte* next, byte* last)
	{
		if ((next >= first) && (next <= last))
		{
			setp(first, last);
			pbump((int)(next - first));
			return true;
		}
		return false;
	}

	void bytestreambuf::updateReadAfterWrite()
	{
		if ((_mode & Write) && (_mode & Read))
		{
			byte* writeNext = pptr();

			byte* readFirst = eback();
			byte* readNext = gptr();
			byte* readLast = writeNext;

			setg(readFirst, readNext, readLast);
		}
	}
}
