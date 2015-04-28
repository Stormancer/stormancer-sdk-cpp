#pragma once
#include <streambuf>
using byte = unsigned char;

class bytebuf : public std::basic_streambuf<byte>
{
public:
	bytebuf();
	bytebuf(byte* begin, byte* end);
	bytebuf(byte* begin, std::streamsize n);
	bytebuf(const bytebuf&);
	bytebuf(bytebuf&&);
	bytebuf& operator=(const bytebuf&);

private:
	std::basic_streambuf<byte>* setbuf(byte* begin, std::streamsize n);
	pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	pos_type seekpos(off_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	int sync();

	std::streamsize showmanyc() const;
	std::streamsize xsgetn(byte* s, std::streamsize n);
	int_type underflow();
	int_type uflow();
	int_type pbackfail(int_type ch);

	std::streamsize xsputn(byte* s, std::streamsize n);
	int_type overflow(int_type c = traits_type::eof());

private:
	byte* _begin = nullptr;
	byte* _end = nullptr;
	byte* _current = nullptr;
};
