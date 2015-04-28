#pragma once
#include "headers.h"
#include "IConnection.h"
#include "Helpers.h"

namespace Stormancer
{
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

			if (stream)
			{
				auto sb = stream->rdbuf();
				sb->pubsetbuf(nullptr, 0);
				delete stream;
			}
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
			return static_cast<TData*>(_metadata[key]);
		}

		void removeMetadata(wstring key)
		{
			auto it = _metadata.find(key);
			_metadata.erase(it);
		}

	public:
		T* connection;
		bytestream* stream;

		Helpers::Action<void> cleanup;

	private:
		anyMap _metadata;
	};
};
