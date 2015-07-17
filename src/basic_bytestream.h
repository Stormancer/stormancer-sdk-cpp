#pragma once
#include "basic_bytebuf.h"
#include "ILogger.h"

namespace Stormancer
{
	namespace Streams
	{
		using namespace std;

		// TEMPLATE CLASS basic_bytestream
		/// Byte array stream.
		template<class _Elem, class _Traits, class _Alloc>
		class basic_bytestream
			: public basic_iostream < _Elem, _Traits >
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
				_Assign_rv(::std::forward<_Myt>(_Right));
			}

			_Myt& operator=(_Myt&& _Right)
			{	// move from _Right
				_Assign_rv(::std::forward<_Myt>(_Right));
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

			virtual ~basic_bytestream() throw()
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

			// I/O OPERATORS

			template<typename T>
			_Myt& operator<<(const T& data)
			{
				std::stringstream ss;
				ss << "#### Write & " << sizeof(T) << std::endl;
				Stormancer::ILogger::instance()->log(ss.str());
#ifdef _IS_BIG_ENDIAN
				Stormancer::ILogger::instance()->log("W1");
				T tmp(data);
				Stormancer::ILogger::instance()->log("W2");
				reverseByteOrder(&tmp);
				Stormancer::ILogger::instance()->log("W3");
				this->write((char*)&tmp, sizeof(T));
#else
				Stormancer::ILogger::instance()->log("W1");
				this->write((const char*)&data, sizeof(T));
#endif
				Stormancer::ILogger::instance()->log("W9");
				return *this;
			}

			template<typename T>
			_Myt& operator<<(T&& data)
			{
				std::stringstream ss;
				ss << "#### Write && " << sizeof(T) << std::endl;
				Stormancer::ILogger::instance()->log(ss.str());
#ifdef _IS_BIG_ENDIAN
				Stormancer::ILogger::instance()->log("W1");
				T tmp(data);
				Stormancer::ILogger::instance()->log("W2");
				reverseByteOrder(&tmp);
				Stormancer::ILogger::instance()->log("W3");
				this->write((char*)&tmp, sizeof(T));
#else
				Stormancer::ILogger::instance()->log("W1");
				this->write((char*)&data, sizeof(T));
#endif
				Stormancer::ILogger::instance()->log("W9");
				return *this;
			}

			_Myt& operator<<(const char* data)
			{
				this->write((char*)data, strlen(data));
				return *this;
			}

			_Myt& operator<<(const std::string& data)
			{
				return ((*this) << data.c_str());
			}

			_Myt& operator<<(const wchar_t* data)
			{
				std::stringstream ss;
				ss << "bytestream << " << data;
				Stormancer::ILogger::instance()->log(ss.str());
				this->write((const char*)data, 2 * wcslen(data));
				return *this;
			}

			_Myt& operator<<(const std::wstring& data)
			{
				std::stringstream ss;
				ss << "bytestream << " << std::string(data.begin(), data.end());
				Stormancer::ILogger::instance()->log(ss.str());
				return (*this) << data.c_str();
			}

			template<typename T>
			_Myt& operator>>(T& target)
			{
				std::stringstream ss;
				ss << "bytestream << " << target << std::endl;
				ss << "#### Read " << sizeof(T);
				Stormancer::ILogger::instance()->log(ss.str());
				Stormancer::ILogger::instance()->log("R1");
				char* tmp = (char*)&target;
				Stormancer::ILogger::instance()->log("R2");
				this->read(tmp, sizeof(T));
				Stormancer::ILogger::instance()->log("R3");
#ifdef _IS_BIG_ENDIAN
				reverseByteOrder(&data);
#endif
				Stormancer::ILogger::instance()->log("R4");
				return (*this);
			}

			_Myt& operator>>(std::string& target)
			{
				std::stringstream ss;
				ss << "bytestream << " << target;
				Stormancer::ILogger::instance()->log(ss.str());
				Stormancer::uint32 len = (Stormancer::uint32)this->rdbuf()->in_avail();
				target.reserve(len);
				char* data = new char[len];
				this->read(data, len);
				target.assign(data, len);
				delete[] data;
				return (*this);
			}

			_Myt& operator>>(std::wstring& target)
			{
				std::stringstream ss;
				ss << "bytestream << wstring(" << std::string(target.begin(), target.end()) << ")";
				Stormancer::ILogger::instance()->log(ss.str());
				Stormancer::uint32 len = (Stormancer::uint32)this->rdbuf()->in_avail();
				Stormancer::uint32 nbChars = len / 2;
				str.reserve(nbChars);
				char* data = new char[len];
				this->read(data, len);
				target.assign((wchar_t*)data, nbChars);
				delete[] data;
				return (*this);
			}

		private:
			_Mysb _Stringbuffer;	// the string buffer
		};
	};

	typedef Streams::basic_bytestream<char, std::char_traits<char>, std::allocator<char> > bytestream;
};
