#include "AuthenticationException.h"

GetCredentialException::GetCredentialException(const char* message) : std::runtime_error(message)
{
	
}