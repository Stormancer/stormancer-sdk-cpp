#pragma once
#include <sstream>
#include "basic_bytebuf.h"

using namespace std;

// TEMPLATE CLASS basic_bytestream
template<class _Elem, class _Traits, class _Alloc>
class basic_bytestream
	: public basic_iostream<_Elem, _Traits>
{	// input/output stream associated with a character array
public:
	typedef basic_bytestream<_Elem, _Traits, _Alloc> _Myt;
	typedef basic_iostream<_Elem, _Traits> _Mybase;
	typedef _Elem char_type;
	typedef _Traits traits_type;
	typedef _Alloc allocator_type;
	typedef typename _Traits::int_type int_type;
	typedef typename _Traits::pos_type pos_type;
	typedef typename _Traits::off_type off_type;
	typedef basic_bytebuf<_Elem, _Traits, _Alloc> _Mysb;
	typedef basic_string<_Elem, _Traits, _Alloc> _Mystr;

	explicit basic_bytestream(ios_base::openmode _Mode =
		ios_base::in | ios_base::out)
		: _Mybase(&_Stringbuffer),
		_Stringbuffer(_Mode)
	{	// construct empty character buffer
	}

	explicit basic_bytestream(const _Mystr& _Str,
		ios_base::openmode _Mode = ios_base::in | ios_base::out)
		: _Mybase(&_Stringbuffer),
		_Stringbuffer(_Str, _Mode)
	{	// construct character buffer from NTCS
	}

	basic_bytestream(_Myt&& _Right)
		: _Mybase(&_Stringbuffer)
	{	// construct by moving _Right
		_Assign_rv(_STD forward<_Myt>(_Right));
	}

	_Myt& operator=(_Myt&& _Right)
	{	// move from _Right
		_Assign_rv(_STD forward<_Myt>(_Right));
		return (*this);
	}

	void _Assign_rv(_Myt&& _Right)
	{	// assign by moving _Right
		if (this != &_Right)
		{	// different, worth moving
			_Stringbuffer.str(_Mystr());
			this->swap(_Right);
		}
	}

	void swap(_Myt& _Right)
	{	// swap with _Right
		if (this != &_Right)
		{	// different, swap base and buffer
			_Mybase::swap(_Right);
			_Stringbuffer.swap(_Right._Stringbuffer);
		}
	}

	basic_bytestream(const _Myt&) = delete;
	_Myt& operator=(const _Myt&) = delete;

	virtual ~basic_bytestream() _NOEXCEPT
	{	// destroy the object
	}

	_Mysb *rdbuf() const
	{	// return pointer to buffer
		return ((_Mysb *)&_Stringbuffer);
	}

	_Mysb *rdbuf(_Mysb *_Strbuf)
	{	// set stream buffer pointer
		_Mysb *_Oldstrbuf = _Stringbuffer;
		_Stringbuffer = _Strbuf;
		//clear();
		return (_Oldstrbuf);
	}

	_Mystr str() const
	{	// return string copy of character array
		return (_Stringbuffer.str());
	}

	void str(const _Mystr& _Newstr)
	{	// replace character array from string
		_Stringbuffer.str(_Newstr);
	}

private:
	_Mysb _Stringbuffer;	// the string buffer
};

typedef basic_bytestream<char, char_traits<char>, allocator<char> > bytestream;
