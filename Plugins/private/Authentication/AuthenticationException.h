#pragma once
#include <stdexcept>

class GetCredentialException : public std::runtime_error
{
public:
	GetCredentialException(const char* message);
};