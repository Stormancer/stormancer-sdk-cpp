#include "stormancer.h"

namespace Stormancer
{
	StringFormat::StringFormat()
	{
	}

	wstring StringFormat::replace(wstring& format, wstring& replacement)
	{
		wstring toFind = L"{" + to_wstring(formatI) + L"}";
		wstring::size_type start = format.find(toFind);
		if (start != wstring::npos)
		{
			wstring::size_type size = toFind.size();
			format.replace(start, size, replacement);
		}
		formatI++;
		return format;
	}

	wstring StringFormat::str()
	{
		return stream.str();
	}

	const wchar_t* StringFormat::c_str()
	{
		return stream.str().c_str();
	}

	StringFormat::operator string()
	{
		return Helpers::to_string(stream.str());
	}

	StringFormat::operator wstring()
	{
		return stream.str();
	}
};
