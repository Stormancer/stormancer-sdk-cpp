#pragma once
#include "headers.h"
#include "Core/IConnection.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	template<typename T = IConnection>
	class Packet
	{
	public:
		Packet(T* source, byteStream* stream)
			: connection(source),
			buffer(*(stream.rdbuf())),
			stream(stream)
		{
		}

		Packet(T* source, byteStream* stream, anyMap& metadata)
			: connection(source),
			buffer(*(stream.rdbuf())),
			stream(stream),
			metadata(metadata)
		{
		}

		Packet(const Packet<T>&) = delete;
		Packet<T>& operator=(const Packet<T>&) = delete;

		virtual ~Packet()
		{
		}

	public:
		anyMap metadata()
		{
			return _metadata;
		}

		template<typename TData>
		void setMetadata(wstring key, TData* data)
		{
			_metadata[key] = (void*)data;
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

		ISerializer* serializer()
		{
			return new MsgPackSerializer;
		}

	public:
		byteBuffer* buffer;
		byteStream* stream;
		T* connection;

	private:
		anyMap _metadata;
	};
};
