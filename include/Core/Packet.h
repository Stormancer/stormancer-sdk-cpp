#pragma once
#include "headers.h"
#include "Core/IConnection.h"
#include "Core/ISerializer.h"
#include <RakNetTypes.h>

namespace Stormancer
{
	template<typename T = IConnection>
	class Packet
	{
	public:
		Packet(T* source, RakNet::Packet* packet)
			: connection(source),
			_packet(packet)
		{
			_data = _packet->data;
			_length = _packet->length;
		}

		Packet(T* source, RakNet::Packet* packet, anyMap& metadata)
			: connection(source),
			_packet(packet),
			metadata(metadata)
		{
		}

		Packet(const Packet<T>&) = delete;
		Packet<T>& operator=(const Packet<T>&) = delete;

		virtual ~Packet()
		{
			if (_packet != nullptr)
			{
				if (server != nullptr)
				{
					server->DeallocatePacket(_packet);
				}
				else
				{
					delete[] _packet->data;
					_packet->data = nullptr;
					delete _packet;
				}
				_packet = nullptr;
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

		ISerializer* serializer()
		{
			return new MsgPackSerializer;
		}

	public:
		T* connection;
		byte* data;
		uint32 length;
		RakNet::RakPeerInterface* server = nullptr;

	private:
		RakNet::Packet* _packet;
		anyMap _metadata;
	};
};
