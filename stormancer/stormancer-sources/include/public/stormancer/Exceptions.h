#pragma once

#include "stormancer/BuildConfig.h"

#include <exception>
#include <stdexcept>
#include <string>

namespace Stormancer
{
	class PointerDeletedException : public std::runtime_error
	{
	public:

		PointerDeletedException()
			: std::runtime_error("pointer_deleted")
		{
		}

		PointerDeletedException(const std::string& message)
			: std::runtime_error(message.c_str())
		{
		}
	};

	// This class does not inherit from runtime_error because of the issues we had with it on unreal and PS4
	class DependencyResolutionException : public std::exception
	{
	public:
		DependencyResolutionException(std::string message)
			: _message(message)
		{}

		const char* what() const noexcept override
		{
			return _message.c_str();
		}

	private:
		std::string _message;
	};
}
