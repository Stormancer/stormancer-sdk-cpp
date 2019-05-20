#pragma once

#include "stormancer/BuildConfig.h"
#include <string>
#include <vector>

namespace Stormancer
{
	/// Join a string vector by using a glue string.
	/// \param vector The vector containing the strings to join.
	/// \param glue A glue string. Default is an empty string.
	std::string vectorJoin(const std::vector<std::string>& vector, const std::string& glue = "");

	std::vector<std::string> stringSplit(const std::string& str, const std::string& separator);

	std::vector<std::string> stringSplit(const std::string& str, const char separator);

	std::string stringTrim(const std::string& str, char ch = ' ');

	/// Split a string to a vector by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	std::vector<std::wstring> wstringSplit(const std::wstring& str, const std::wstring& separator);

	std::vector<std::wstring> wstringSplit(const std::wstring& str, const wchar_t separator);

	/// Trim a specific character from a string.
	/// \param str The string to trim.
	/// \param ch the character to remove from the string. Default is space.
	std::wstring wstringTrim(const std::wstring& str, wchar_t ch = ' ');
}
