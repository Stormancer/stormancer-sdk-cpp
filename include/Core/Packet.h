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
		Packet(shared_ptr<T>& source, byteStream& stream)
			: connection(source),
			buffer(*(stream.rdbuf())),
			stream(stream)
		{
		}

		Packet(shared_ptr<T>& source, byteStream& stream, anyMap& metadata)
			: connection(source),
			buffer(*(stream.rdbuf())),
			stream(stream),
			metadata(metadata)
		{
		}

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

		shared_ptr<ISerializer> serializer()
		{
			return make_shared<ISerializer>(new MsgPackSerializer);
		}

	public:
		byteBuffer& buffer;
		byteStream& stream;
		shared_ptr<T> connection;

	private:
		anyMap _metadata;
	};
};
