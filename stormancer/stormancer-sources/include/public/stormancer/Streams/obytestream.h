#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Streams/bytestreambuf.h"
#include <ostream> 
#include <vector>
#include <iostream>

namespace Stormancer
{
	class obytestream : public std::basic_ostream<char>
	{
	public:

#pragma region public_methods

		obytestream();

		obytestream(byte* dataPtr, std::streamsize dataSize, bool allowExtend = false);

		virtual obytestream& operator<<(const char value);

		virtual obytestream& operator<<(const int8 value);

		virtual obytestream& operator<<(const uint8 value);

		virtual obytestream& operator<<(const int16 value);

		virtual obytestream& operator<<(const uint16 value);

		virtual obytestream& operator<<(const int32 value);

		virtual obytestream& operator<<(const uint32 value);

		virtual obytestream& operator<<(const int64 value);

		virtual obytestream& operator<<(const uint64 value);

		virtual obytestream& operator<<(const float32 value);

		virtual obytestream& operator<<(const float64 value);

		virtual obytestream& operator<<(const bool value);

		virtual obytestream& operator<<(const void* value);

		virtual obytestream& operator<<(const std::vector<byte>& bytes);

		virtual std::vector<byte> bytes();

		byte* startPtr();

		byte* currentPtr();

		byte* endPtr();

		std::streamsize totalSize();

		std::streamsize availableSize();

		std::streamsize currentPosition();

		void dynamic(bool dyn);

		obytestream& write(const byte* ptr, std::streamsize size);

		obytestream& write(const char* ptr, std::streamsize size);

#pragma endregion

	private:

#pragma region private_methods

		template<typename T>
		inline obytestream& serialize(const T& value)
		{
			if (good())
			{
				write(reinterpret_cast<const char*>(&value), sizeof(T));
			}
			return (*this);
		}

#pragma endregion

#pragma region private_members

		bytestreambuf _buffer;

#pragma endregion
	};
}
