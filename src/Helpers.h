#pragma once
#include "headers.h"
#include "basic_bytebuf.h"
#include "basic_bytestream.h"

namespace Stormancer
{
	template<typename T = void>
	class Result
	{
	private:
		T _data;
		int _error = -1;
		char* _reason = nullptr;


	public:
		Result()
		{
		}

		Result(T data)
			: _data(data),
			_error(0)
		{
		}

		~Result()
		{
			if (_reason)
			{
				delete[] _reason;
				_reason = nullptr;
			}
		}

		bool finished()
		{
			return (_error != -1);
		}

		bool success()
		{
			return (_error == 0);
		}

		void set(T data)
		{
			_data = data;
			_error = 0;
		}

		void setError(int error, const char* reason)
		{
			_error = error;

			if (_reason)
			{
				delete[] _reason;
				_reason = nullptr;
			}

			auto sz = std::strlen(reason) + 1;
			_reason = new char[sz];
			std::memcpy(_reason, reason, sz);
		}

		int error()
		{
			return _error;
		}

		const char* reason()
		{
			return _reason;
		}

		T get()
		{
			return _data;
		}

		void destroy()
		{
			delete this;
		}
	};

	template<>
	class Result<void>
	{
	private:
		int _error = -1;
		char* _reason = nullptr;

	public:
		Result()
		{
		}

		~Result()
		{
			if (_reason)
			{
				delete[] _reason;
				_reason = nullptr;
			}
		}

		bool finished()
		{
			return (_error != -1);
		}

		bool success()
		{
			return (_error == 0);
		}

		void set()
		{
			_error = 0;
		}

		void setError(int error, const char* reason)
		{
			_error = error;

			if (_reason)
			{
				delete[] _reason;
				_reason = nullptr;
			}

			auto sz = std::strlen(reason) + 1;
			_reason = new char[sz];
			std::memcpy(_reason, reason, sz);
		}

		int error()
		{
			return _error;
		}

		const char* reason()
		{
			return _reason;
		}

		void destroy()
		{
			delete this;
		}
	};

	template<typename T>
	class MsgPackMaybe
	{
	private:
		T* _t;

	public:
		MsgPackMaybe(T* t = nullptr)
			: _t(t)
		{
		}

		MsgPackMaybe(const MsgPackMaybe& mb)
			: _t(mb._t)
		{
		}

		MsgPackMaybe& operator=(const MsgPackMaybe& mb)
		{
			_t = mb._t;
			return *this;
		}

		virtual ~MsgPackMaybe()
		{
		}

		MsgPackMaybe& operator=(T* t)
		{
			_t = t;
			return *this;
		}

		bool hasValue()
		{
			return (_t ? true : false);
		}

		T* get()
		{
			return _t;
		}

		template<typename Packer>
		void msgpack_pack(Packer& pk) const
		{
			if (_t)
			{
				pk.pack(*_t);
			}
			else
			{
				pk.pack_nil();
			}
		}

		void msgpack_unpack(msgpack::object const& o)
		{
			if (o.is_nil())
			{
				_t = nullptr;
			}
			else
			{
				_t = new T();
				msgpack::adaptor::convert<T>()(o, *_t);
			}
		}

		void destroy()
		{
			if (_t)
			{
				delete _t;
				_t = nullptr;
			}
		}
	};

	template<typename T>
	struct deleter
	{
		void operator()(const T* p)
		{
			delete p;
		}
	};

	template<typename T>
	struct array_deleter
	{
		void operator()(const T* p)
		{
			delete[] p;
		}
	};

#pragma region flux

	// vector flux operators

	template<typename T>
	std::vector<T>& operator<<(std::vector<T>& v, const T& data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	std::vector<T>& operator<<(std::vector<T>& v, const T&& data)
	{
		v.push_back(data);
		return v;
	}

	template<typename T>
	std::vector<T>& operator>>(std::vector<T>& v, T& data)
	{
		data = v.pop_back();
		return v;
	}

	template<typename T>
	std::string to_string(T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	template<typename T>
	T* reverseByteOrder(T* data, uint64 n = 0)
	{
		char* tmp = (char*)data;
		if (n == 0)
		{
			n = sizeof(T);
		}
		std::reverse(tmp, tmp + n);
		return data;
	}

#pragma endregion

#pragma region map

	anyMap stringMapToAnyMap(stringMap& sm);

	template<typename TKey, typename TValue>
	std::vector<TKey> mapKeys(std::map<TKey, TValue>& map)
	{
		std::vector<TKey> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->first);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	std::vector<TValue> mapValues(std::map<TKey, TValue>& map)
	{
		std::vector<TValue> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.push_back(it->second);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TKey> RakMapKeys(DataStructures::Map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		uint32 sz = map.Size();
		for (uint32 i = 0; i < sz; ++i)
		{
			container.Insert(map.GetKeyAtIndex(i), __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TValue> RakMapValues(DataStructures::Map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		uint32 sz = map.Size();
		for (uint32 i = 0; i < sz; ++i)
		{
			container.Insert(map[i], __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TKey> mapToRakListKeys(std::map<TKey, TValue>& map)
	{
		DataStructures::List<TKey> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.Insert(it->first, __FILE__, __LINE__);
		}
		return container;
	}

	template<typename TKey, typename TValue>
	DataStructures::List<TValue> mapToRakListValues(std::map<TKey, TValue>& map)
	{
		DataStructures::List<TValue> container;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			container.Insert(it->second, __FILE__, __LINE__);
		}
		return container;
	}

	/// Returns a boolean indicating if the map contains the key.
	template<typename TKey, typename TValue>
	bool mapContains(const std::map<TKey, TValue>& map, const TKey& key)
	{
		return (map.find(key) != map.end()) ? true : false;
	}

#pragma endregion

#pragma region string

	/// Join a string vector by using a glue string.
	/// \param vector The vector containing the strings to join.
	/// \param glue A glue string. Default is an empty string.
	STORMANCER_DLL_API std::string vectorJoin(std::vector<std::string>& vector, std::string glue = "");

	/// Split a string to a vector by using a separator string.
	/// \param str The string to split.
	/// \param separator the separator to detect in the string.
	STORMANCER_DLL_API std::vector<std::wstring> stringSplit(const std::wstring& str, const std::wstring separator);

	/// Trim a specific character from a string.
	/// \param str The string to trim.
	/// \param ch the character to remove from the string. Default is space.
	STORMANCER_DLL_API std::wstring stringTrim(std::wstring& str, wchar_t ch = ' ');

#pragma endregion

#pragma region task

	pplx::task<void> taskCompleted();

	template<typename T>
	pplx::task<T> taskCompleted(T& result)
	{
		pplx::task_completion_event<T> tce;
		tce.set(result);
		return pplx::create_task(tce);
	}

	template<typename T>
	pplx::task<T> taskFromException(const std::exception& e)
	{
		pplx::task_completion_event<T> tce;
		tce.set_exception<std::exception>(e);
		return create_task(tce);
	}

	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

	template<typename T>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>()> func)
	{
		try
		{
			return func();
		}
		catch (const std::exception& e)
		{
			return taskFromException<T>(e);
		}
	}

	template<typename T, typename U>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>(U)> func, U& argument)
	{
		try
		{
			return func(argument);
		}
		catch (const std::exception& e)
		{
			return taskFromException<T>(e);
		}
	}

#pragma endregion

#pragma region stream

	//bytestream* convertRakNetPacketToStream(RakNet::Packet* packet);

	//void deleteStringBuf(std::stringbuf* sb);

	template<typename T, typename U>
	void streamCopy(T* fromStream, U* toStream)
	{
		uint32 n = static_cast<uint32>(fromStream->rdbuf()->in_avail());
		char* c = new char[n];
		fromStream->readsome(c, n);
		toStream->write(c, n);
		delete[] c;
	}

	STORMANCER_DLL_API char* readToEnd(bytestream* stream, std::streamsize* length);

#pragma endregion

#pragma region time

	STORMANCER_DLL_API time_t nowTime_t();
	STORMANCER_DLL_API std::string time_tToStr(time_t& time, bool local = false);
	STORMANCER_DLL_API std::string time_tToStr(time_t& time, const char* format);
	STORMANCER_DLL_API std::string nowStr(bool local = false);
	STORMANCER_DLL_API std::string nowStr(const char* format);
	STORMANCER_DLL_API std::string nowDateStr(bool local = false);
	STORMANCER_DLL_API std::string nowTimeStr(bool local = false);
	void writetime(std::ostream &os, std::time_t tc, const char* format); // alternative to std::put_time (not in libstdc++4.9)

#pragma endregion

#pragma region other

	bool ensureSuccessStatusCode(int statusCode);

	template<typename T>
	uint64 ptrToUint64(T* ptr)
	{
		return *static_cast<uint64*>(static_cast<void*>(ptr));
	}

	std::string stringifyBytesArray(std::string bytes, bool hex = false);

#pragma endregion
};
