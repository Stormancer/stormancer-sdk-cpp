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
			setbuf(reinterpret_cast<char*>(dataPtr), dataSize);
		}
		dynamic(isDynamic);
	}

	bytestreambuf::~bytestreambuf()
	{
		tidy();
	}

	byte* bytestreambuf::startReadPtr()
	{
		return reinterpret_cast<byte*>(eback());
	}

	byte* bytestreambuf::currentReadPtr()
	{
		return reinterpret_cast<byte*>(gptr());
	}

	byte* bytestreambuf::endReadPtr()
	{
		return reinterpret_cast<byte*>(egptr());
	}

	byte* bytestreambuf::startWritePtr()
	{
		return reinterpret_cast<byte*>(pbase());
	}

	byte* bytestreambuf::currentWritePtr()
	{
		return reinterpret_cast<byte*>(pptr());
	}

	byte* bytestreambuf::endWritePtr()
	{
		return reinterpret_cast<byte*>(epptr());
	}

	std::streamsize bytestreambuf::size()
	{
		if (_mode & Write)
		{
			char* first = eback();
			char* last = egptr();
			if (first && last)
			{
				return (last - first);
			}
		}
		else if (_mode & Read)
		{
			char* first = pbase();
			char* last = epptr();
			if (first && last)
			{
				return (last - first);
			}
		}
		return 0;
	}

	std::streamsize bytestreambuf::currentReadPosition()
	{
		char* first = eback();
		char* next = gptr();
		if (first && next)
		{
			return (next - first);
		}
		return 0;
	}

	std::streamsize bytestreambuf::currentWritePosition()
	{
		char* first = pbase();
		char* next = pptr();
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

	std::basic_streambuf<char>* bytestreambuf::setbuf(char* s, std::streamsize size)
	{
		tidy();
		setg(s, s, (s + size));
		setp(s, (s + size));
		_mode = (Read | Write);
		return (this);
	}

	bytestreambuf::pos_type bytestreambuf::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
	{
		pos_type pt = -1;

		if ((_mode & Read) && (which & std::ios_base::in))
		{
			char* first = eback();
			char* next = gptr();
			char* last = egptr();

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
			char* first = pbase();
			char* next = pptr();
			char* last = epptr();

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
			char* first = eback();
			char* next = first + (int)pos;
			char* last = egptr();

			if (setgIfValid(first, next, last))
			{
				pt = pos;
			}
		}

		if ((_mode & Write) && (which & std::ios_base::out))
		{
			char* first = pbase();
			char* next = first + (int)pos;
			char* last = epptr();

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
			char* next = gptr();
			char* last = egptr();
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

	std::streamsize bytestreambuf::xsgetn(char* s, std::streamsize n)
	{
		std::streamsize res = 0;

		if (s)
		{
			if (_mode & Read)
			{
				std::streamsize size = std::min(showmanyc(), n);
				char* first = eback();
				char* next = gptr();
				char* last = egptr();
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
			char* next = gptr();
			char* last = egptr();
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
			char* next = gptr();
			char* last = egptr();
			if (next != last)
			{
				char* first = eback();
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
			char* first = eback();
			char* next = gptr();
			if (next > first)
			{
				char* last = egptr();
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
				char* first = pbase();
				char* next = pptr();
				char* last = epptr();
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
					std::memcpy(next, s, (std::size_t)sz);
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
			char* next = pptr();
			char* last = epptr();
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
			char* ptr = pbase();

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
		char* ptr;
		char* ptrWriteLast;
		if (size > 0)
		{
			ptr = new char[(unsigned int)size]();
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

			char* oldReadFirst = eback();
			char* oldReadNext = gptr();
			char* oldReadLast = egptr();

			char* oldWriteFirst = pbase();
			char* oldWriteNext = pptr();
			char* oldWriteLast = epptr();

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
				char* writeFirst = pbase();
				char* writeNext = pptr();
				char* writeLast = epptr();

				std::memcpy(writeFirst, oldWriteFirst, (std::size_t)oldSize);

				writeNext = writeFirst + (oldWriteNext - oldWriteFirst);

				setp(writeFirst, writeLast);
				pbump((int)(oldWriteNext - oldWriteFirst));

				char* readFirst = (oldReadFirst ? writeFirst : nullptr);
				char* readNext = (oldReadNext ? writeFirst + (oldReadNext - oldReadFirst) : nullptr);
				char* readLast = (oldReadLast ? writeLast : nullptr);

				setg(readFirst, readNext, readLast);
			}

			if ((oldMode & Allocated) && oldWriteFirst)
			{
				delete[] oldWriteFirst;
			}
		}
	}

	inline bool bytestreambuf::setgIfValid(char* first, char* next, char* last)
	{
		if ((next >= first) && (next <= last))
		{
			setg(first, next, last);
			return true;
		}
		return false;
	}

	inline bool bytestreambuf::setpIfValid(char* first, char* next, char* last)
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
			char* writeNext = pptr();

			char* readFirst = eback();
			char* readNext = gptr();
			char* readLast = writeNext;

			setg(readFirst, readNext, readLast);
		}
	}
}
