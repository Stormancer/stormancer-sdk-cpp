#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/stormancerTypes.h"
#include <streambuf>


namespace Stormancer
{
	class bytestreambuf : public std::basic_streambuf<byte>
	{
	public:

#pragma region public_methods

		enum
		{
			Read = 1,
			Write = 2,
			Dynamic = 4,
			Allocated = 8
		};
		typedef int Mode;

		bytestreambuf();

		bytestreambuf(byte* dataPtr, std::streamsize dataSize, bool isDynamic = false);

		~bytestreambuf();

		byte* startReadPtr();

		byte* currentReadPtr();

		byte* endReadPtr();

		byte* startWritePtr();

		byte* currentWritePtr();

		byte* endWritePtr();

		std::streamsize size();

		std::streamsize currentReadPosition();

		std::streamsize currentWritePosition();

		void dynamic(bool dyn);

#pragma endregion

	protected:

#pragma region protected_methods

		std::basic_streambuf<byte>* setbuf(byte* s, std::streamsize size) override;

		pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;

		pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;

		std::streamsize showmanyc() override;

		std::streamsize xsgetn(byte* s, std::streamsize n) override;

		int_type underflow() override;

		int_type uflow() override;

		int_type pbackfail(int_type c = traits_type::eof()) override;

		std::streamsize xsputn(const char_type* s, std::streamsize n) override;

		int_type overflow(int_type c = traits_type::eof()) override;

		void tidy();

		void reset();

		void init(std::streamsize size = 0);

		void grow(std::streamsize minSize);

		inline bool setgIfValid(byte* first, byte* next, byte* last);

		inline bool setpIfValid(byte* first, byte* next, byte* last);

		void updateReadAfterWrite();

#pragma endregion

	private:

#pragma region private_members

		Mode _mode = 0;

#pragma endregion
	};
}
