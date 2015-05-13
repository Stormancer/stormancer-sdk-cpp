#pragma once
#include "headers.h"

namespace Stormancer
{
	class StringFormat
	{
	public:
		StringFormat();

		template<typename... Args>
		StringFormat(wstring format, Args&... args)
		{
			int _[] = { 0, (format = replace(format, args), 0)... };
			stream << format;
		}

	public:
		wstring str();

		const wchar_t* c_str();

		operator string();
		operator wstring();

		template<typename T>
		wstring replace(wstring& format, T& replacement)
		{
			return replace(format, Helpers::to_wstring(replacement));
		}

		wstring replace(wstring& format, wstring& replacement);

		template<typename T>
		StringFormat& operator<<(T& data)
		{
			stream << data;
			return *this;
		}

		template<typename T>
		void operator>>(T& data)
		{
			stream >> data;
		}

	private:
		wstringstream stream;
		int formatI = 0;
	};
};
