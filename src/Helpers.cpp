#include "stormancer.h"

namespace Stormancer
{
	bytestream* Helpers::convertRakNetBitStreamToByteStream(RakNet::BitStream* stream)
	{
		auto bs = new bytestream;
		if (stream != nullptr)
		{
			auto buf = bs->rdbuf();
			char* data = static_cast<char*>(static_cast<void*>(stream->GetData()));
			buf->pubsetbuf(data, stream->GetNumberOfBytesUsed());
			stream->SetData(nullptr);
			stream->SetNumberOfBitsAllocated(0);
			RakNet::BitStream::DestroyInstance(stream);
			delete stream;
		}
		return bs;
	}

	template<>
	vector<byte> Helpers::convert<string, vector<byte>>(string& str)
	{
		vector<byte> v;
		v.resize(str.size());
		for (uint32 i = 0; v.size(); i++)
		{
			v[i] = str[i];
		}
		return v;
	}

	stringstream* Helpers::convertRakNetPacketToStringStream(RakNet::Packet* packet, RakNet::RakPeerInterface* peer)
	{
		auto ss = new stringstream;
		if (packet != nullptr)
		{
			auto sb = ss->rdbuf();
			char* data = static_cast<char*>(static_cast<void*>(packet->data));
			sb->pubsetbuf(data, packet->length);
		}
		return ss;
	}

	bool Helpers::ensureSuccessStatusCode(int statusCode)
	{
		return (statusCode >= 200 && statusCode < 300);
	}

	void Helpers::displayException(const exception& e)
	{
		wcout << Helpers::StringFormat(L"Exception: {0}", string(e.what())).c_str() << endl;
	}

	anyMap Helpers::stringMapToAnyMap(stringMap& sm)
	{
		anyMap am;
		for (auto it = sm.begin(); it != sm.end(); ++it)
		{
			am[it->first] = (void*)&it->second;
		}
		return am;
	}

	wstring Helpers::vectorJoin(vector<wstring> vector, wstring glue)
	{
		wstringstream ss;
		for (size_t i = 0; i < vector.size(); ++i)
		{
			if (i != 0)
			{
				ss << glue;
			}
			ss << vector[i];
		}
		return ss.str();
	}

	vector<wstring> Helpers::stringSplit(const wstring& str, const wstring separator)
	{
		vector<wstring> splitted;
		size_t cursor = 0, lastCursor = 0;
		while ((cursor = str.find(separator, cursor)) != wstring::npos)
		{
			splitted << str.substr(lastCursor, cursor - lastCursor);
			lastCursor = cursor;
			cursor++;
		}
		splitted << str.substr(lastCursor + 1, str.length() - lastCursor);
		return splitted;
	}

	wstring Helpers::stringTrim(wstring& str, wchar_t ch)
	{
		function<int(int)> ischar = [ch](int c) -> int {
			if (c == ch)
			{
				return 1;
			}
			return 0;
		};
		str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ischar)));
		str.erase(find_if(str.rbegin(), str.rend(), not1(ischar)).base(), str.end());
		return str;
	};

	pplx::task<void> Helpers::taskCompleted()
	{
		pplx::task_completion_event<void> tce;
		tce.set();
		return create_task(tce);
	}

	pplx::task<void> Helpers::taskIf(bool condition, function<pplx::task<void>()> action)
	{
		if (condition)
		{
			return action();
		}
		else
		{
			return taskCompleted();
		}
	}

	time_t Helpers::nowTime_t()
	{
		return chrono::system_clock::to_time_t(chrono::system_clock::now());
	}

	wstring Helpers::time_tToStr(time_t& time, bool local)
	{
		struct tm timeinfo;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&timeinfo, &time);
#else
		localtime_r(&time, &timeinfo); // POSIX
#endif
		const char* format = local ? "%x %X" : "%Y-%m-%dT%H:%M:%SZ";
		stringstream ss;
		ss << put_time(const_cast<tm*>(&timeinfo), format);
		return Helpers::to_wstring(ss.str());
	}

	wstring Helpers::nowStr(bool local)
	{
		time_t now = nowTime_t();
		return time_tToStr(now, local);
	}

	template<>
	wstring Helpers::to_wstring<string>(string str)
	{
		return wstring(str.begin(), str.end());
	}

	template<>
	wstring Helpers::to_wstring<const char*>(const char* str)
	{
		return to_wstring(string(str));
	}

	template<>
	string Helpers::to_string<wstring>(wstring& str)
	{
		return string(str.begin(), str.end());
	}

	template<>
	string Helpers::to_string<vector<byte>>(vector<byte>& v)
	{
		string str;
		str.resize(v.size());
		for (size_t i = 0; i < v.size(); i++)
		{
			str[i] = v[i];
		}
		return str;
	}

	Helpers::StringFormat::StringFormat()
	{
	}

	wstring Helpers::StringFormat::str()
	{
		return stream.str();
	}

	const wchar_t* Helpers::StringFormat::c_str()
	{
		return stream.str().c_str();
	}

	Helpers::StringFormat::operator string()
	{
		return to_string(stream.str());
	}

	Helpers::StringFormat::operator wstring()
	{
		return stream.str();
	}

	template<>
	wstring Helpers::StringFormat::replace<wstring>(wstring& format, wstring& replacement)
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
};
