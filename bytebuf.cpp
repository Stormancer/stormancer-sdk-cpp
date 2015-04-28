#pragma once
#include "bytebuf.h"
#include <functional>
#include <cassert>
#include <cstring>
#include <algorithm>

bytebuf::bytebuf()
{
}

bytebuf::bytebuf(byte* begin, byte* end)
	: _begin(begin),
	_end(end),
	_current(_begin)
{
	assert(std::less_equal<byte*>()(_begin, _end));
}

bytebuf::bytebuf(byte* begin, std::streamsize n)
{
	setbuf(begin, n);
}

bytebuf::bytebuf(const bytebuf& other)
{
	*this = other;
}

bytebuf::bytebuf(bytebuf&& right)
{
	byte* tbegin = _begin;
	byte* tend = _end;
	byte* tcurrent = _current;

	_begin = right._begin;
	_end = right._end;
	_current = right._current;

	right._begin = tbegin;
	right._end = tend;
	right._current = tcurrent;
}

bytebuf& bytebuf::operator=(const bytebuf& other)
{
	auto n = other.showmanyc();
	_begin = new byte[n];
	std::memcpy(_begin, other._begin, n);
	_end = _begin + n;
	_current = other._current;
}

std::basic_streambuf<byte>* bytebuf::setbuf(byte* begin, std::streamsize n)
{
	_begin = begin;
	_end = _begin + n;
	_current = _begin;
}

bytebuf::pos_type bytebuf::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
{
	byte* newpos;

	if (way == std::ios_base::beg)
	{
		newpos = _begin + off;
	}
	else if (way == std::ios_base::cur)
	{
		newpos = _current + off;
	}
	else if (way == std::ios_base::end)
	{
		newpos = _end + off;
	}

	if (newpos >= _begin && newpos <= _end)
	{
		_current = newpos;
		return pos_type(off_type(_current));
	}

	return -1;
}

bytebuf::pos_type bytebuf::seekpos(off_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
{
	byte* newpos = _begin + pos;

	if (newpos >= _begin && newpos <= _end)
	{
		_current = newpos;
		return pos_type(off_type(_current));
	}

	return -1;
}

int bytebuf::sync()
{
	return 0;
}

std::streamsize bytebuf::showmanyc() const
{
	return _end - _current;
}

std::streamsize bytebuf::xsgetn(byte* s, std::streamsize n)
{
	n = std::min(n, showmanyc());
	memcpy(s, _current, n);
	_current += n;
	return n;
}

bytebuf::int_type bytebuf::underflow()
{
	if (_current == _end)
	{
		return traits_type::eof();
	}

	return traits_type::to_int_type(*_current);
}

bytebuf::int_type bytebuf::uflow()
{
	if (_current == _end)
	{
		return traits_type::eof();
	}

	return traits_type::to_int_type(*_current++);
}

bytebuf::int_type bytebuf::pbackfail(int_type ch)
{
	if (_current == _begin || (ch != traits_type::eof() && ch != _current[-1]))
	{
		return traits_type::eof();
	}

	return traits_type::to_int_type(*--_current);
}

std::streamsize bytebuf::xsputn(byte* s, std::streamsize n)
{
	n = std::min(n, showmanyc());
	memcpy(_current, s, n);
	_current += n;
	return n;
}

bytebuf::int_type bytebuf::overflow(int_type c = traits_type::eof())
{
	if (c != traits_type::eof() && _current && _current >= _begin && _current <= _end)
	{
		(*_current) = c;
		return traits_type::to_int_type(c);
	}

	return traits_type::eof();
}
