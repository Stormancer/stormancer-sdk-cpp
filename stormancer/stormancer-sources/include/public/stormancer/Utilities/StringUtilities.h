#pragma once

#include "stormancer/BuildConfig.h"
#include <string>
#include <vector>

namespace Stormancer
{
	/// Join a vector of strings by using a glue string.
	/// \param vector The vector of strings to join.
	/// \param glue A glue string. Default is empty string.
	std::string stringJoin(const std::vector<std::string>& vector, const std::string& glue = "");

	/// Join a vector of strings by using a glue string.
	/// \param vector The vector of strings to join.
	/// \param glue A glue string. Default is empty string.
	std::wstring stringJoin(const std::vector<std::wstring>& vector, const std::wstring& glue = L"");

	/// Split a string to a vector of strings by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::string> stringSplit(const std::string& str, const std::string& separator);

	/// Split a string to a vector of strings by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::string> stringSplit(const std::string& str, const char separator);

	/// Split a string to a vector of strings by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::wstring> stringSplit(const std::wstring& str, const std::wstring& separator);

	/// Split a string to a vector of strings by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::wstring> stringSplit(const std::wstring& str, const wchar_t separator);

	/// Trim a specific character from the start and the end of a string.
	/// \param str The string to trim.
	/// \param ch the character to remove from the string. Default is space.
	std::string stringTrim(const std::string& str, char ch = ' ');

	/// Trim a specific character from the start and the end of a string.
	/// \param str The string to trim.
	/// \param ch the character to remove from the string. Default is space.
	std::wstring stringTrim(const std::wstring& str, wchar_t ch = ' ');
}
