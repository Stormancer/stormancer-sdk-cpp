#include "stdafx.h"
#include "Streams/bytestreambuf.h"

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

	byte* bytestreambuf::ptr()
	{
		return pbase();
	}

	int bytestreambuf::gcount()
	{
		byte* first = eback();
		byte* last = gptr();
		if (first && last)
		{
			return (int)(last - first);
		}
		return 0;
	}

	int bytestreambuf::pcount()
	{
		byte* first = pbase();
		byte* next = pptr();
		if (first && next)
		{
			return (int)(next - first);
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
		setg(s, s, s + size);
		setp(s, s, s + size);
		_mode = (Read | Write);
		return (this);
	}

	bytestreambuf::pos_type bytestreambuf::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
	{
		pos_type pt = -1;

		if ((_mode & Read) && (which & std::ios_base::in))
		{
			byte* first = eback();
			byte* next = 0;
			byte* last = egptr();

			if (way == std::ios_base::beg)
			{
				next = first + off;
			}
			else if (way == std::ios_base::cur)
			{
				next = next + off;
			}
			else if (way == std::ios_base::end)
			{
				next = last - off;
			}

			if (setgIfValid(first, next, last))
			{
				pt = (next - first);
			}
		}

		if ((_mode & Write) && (which & std::ios_base::out))
		{
			byte* first = pbase();
			byte* next = 0;
			byte* last = epptr();

			if (way == std::ios_base::beg)
			{
				next = first + off;
			}
			else if (way == std::ios_base::cur)
			{
				next = next + off;
			}
			else if (way == std::ios_base::end)
			{
				next = last - off;
			}

			if (setpIfValid(first, next, last))
			{
				pt = (next - first);
			}
		}

		return pt;
	}

	bytestreambuf::pos_type bytestreambuf::seekpos(pos_type pos, std::ios_base::openmode which)
	{
		pos_type pt = -1;

		if ((_mode & Read) && (which & std::ios_base::in))
		{
			byte* first = eback();
			byte* next = first + (int)pos;
			byte* last = egptr();

			if (setgIfValid(first, next, last))
			{
				pt = pos;
			}
		}

		if ((_mode & Write) && (which & std::ios_base::out))
		{
			byte* first = pbase();
			byte* next = first + (int)pos;
			byte* last = epptr();

			if (setpIfValid(first, next, last))
			{
				pt = pos;
			}
		}

		return pt;
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
				std::memcpy(s, next, (std::size_t)size);
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

		if (s)
		{
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
				if (avail < n)
				{
					if (_mode & Dynamic)
					{
						grow((next - first) + n);
						first = pbase();
						next = pptr();
						last = epptr();
						avail = last - next;
					}
				}
				std::streamsize sz = std::min(avail, n);
				if (sz > 0)
				{
					std::memcpy(next, s, (std::size_t)sz);
					next += sz;
					setp(first, next, last);
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
		setp(nullptr, nullptr, nullptr);
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
		setp(ptr, ptr, ptrWriteLast);
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

				std::memcpy(writeFirst, oldWriteFirst, (std::size_t)oldSize);

				writeNext = writeFirst + (oldWriteNext - oldWriteFirst);

				setp(writeFirst, writeNext, writeLast);

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
			setp(first, next, last);
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
