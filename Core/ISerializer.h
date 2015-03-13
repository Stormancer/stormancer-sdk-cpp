#pragma once

#include <iostream>

class ISerializer
{
public:
	ISerializer() {};
	//virtual ~ISerializer() = 0;

	//template<typename T>
	//virtual void serialize(T data, std::istream stream) {};

	//template<typename T>
	//virtual T deserialize<T>(std::ostream stream) {};

public:
	const std::string name;
};
