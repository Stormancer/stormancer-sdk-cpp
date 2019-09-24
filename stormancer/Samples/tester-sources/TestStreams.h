#pragma once

#include "stormancer/Streams/bytestream.h"
#include <array>
#include <cstring>

class TestStreams
{
public:
	static void runTests()
	{
		std::array<int, 10> myArray;
		myArray.fill(10);

		Stormancer::ibytestream input(reinterpret_cast<Stormancer::byte*>(myArray.data()), myArray.size()*sizeof(int));

		auto inputBytes = input.bytes();
		assertex(inputBytes.size() * sizeof(Stormancer::byte) == myArray.size() * sizeof(int), "The data from ibytestream::bytes() should have the same size as the original array");
		assertex(std::memcmp(inputBytes.data(), myArray.data(), myArray.size() * sizeof(int)) == 0, "The data from ibytestream::bytes() should be the same as in the original array");

		Stormancer::obytestream output;
		output << input;

		auto outputBytes = output.bytes();
		assertex(outputBytes.size() * sizeof(Stormancer::byte) == myArray.size() * sizeof(int), "The data from ibytestream::bytes() should have the same size as the original array");
		assertex(std::memcmp(outputBytes.data(), myArray.data(), myArray.size() * sizeof(int)) == 0, "The data from ibytestream::bytes() should be the same as in the original array");

		assertex(input.availableSize() == 0, "The ibytestream should be empty after output << input");

		input.seekg(0, std::ios_base::beg);
		inputBytes = input.bytes();
		assertex(inputBytes.size() * sizeof(Stormancer::byte) == myArray.size() * sizeof(int), "The data from ibytestream::bytes() after rewinding should have the same size as the original array");
		assertex(std::memcmp(inputBytes.data(), myArray.data(), myArray.size() * sizeof(int)) == 0, "The data from ibytestream::bytes() after rewinding should be the same as in the original array");
		
		inputBytes.clear();
		input >> inputBytes;
		assertex(inputBytes.size() * sizeof(Stormancer::byte) == myArray.size() * sizeof(int), "The data from ibytestream::>>(vector) after rewinding should have the same size as the original array");
		assertex(std::memcmp(inputBytes.data(), myArray.data(), myArray.size() * sizeof(int)) == 0, "The data from ibytestream::>>(vector) after rewinding should be the same as in the original array");
	}

private:
	static void assertex(bool condition, std::string message)
	{
		if (!condition)
		{
			throw std::runtime_error(message.c_str());
		}
	}
};