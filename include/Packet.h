#pragma once
#include "headers.h"
#include "IConnection.h"
#include "Helpers.h"
#include "Action.h"

namespace Stormancer
{
	class Request;

	/// Expose a stream for reading data received through the network.
	template<typename T = IConnection>
	class Packet
	{
	public:
		Packet(T* source, bytestream* stream)
			: connection(source),
			stream(stream)
		{
		}

		Packet(T* source, bytestream* stream, anyMap& metadata)
			: connection(source),
			stream(stream),
			_metadata(metadata)
		{
		}

		Packet(const Packet<T>&) = delete;
		Packet<T>& operator=(const Packet<T>&) = delete;

		virtual ~Packet()
		{
			cleanup();
		}

	public:
		anyMap metadata()
		{
			return _metadata;
		}

		template<typename TData>
		void setMetadata(wstring key, TData* data)
		{
			_metadata[key] = static_cast<void*>(data);
		}

		template<typename TData>
		TData* getMetadata(wstring key)
		{
			return (TData*)_metadata[key];
		}

		void removeMetadata(wstring key)
		{
			auto it = _metadata.find(key);
			_metadata.erase(it);
		}

	public:
		T* connection = nullptr;
		bytestream* stream = nullptr;

		shared_ptr<Request> request = nullptr;

		Action<> cleanup;

	private:
		anyMap _metadata;
	};
};
