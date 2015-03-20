#include "Core/Packet.h"

namespace Stormancer
{
	// Packet

	template<typename T>
	Packet::Packet(shared_ptr<T*> source, byteStream& stream, StringMap metadata)
		: connection(source),
		stream(stream),
		metadata(metadata)
	{
	}

	template<typename T>
	Packet::~Packet()
	{
	}

	template<typename T>
	template<typename TData>
	void Packet::setMetadata(string key, TData* data)
	{
		metadata[key] = data;
	}

	template<typename T>
	template<typename TData>
	TData* Packet::getMetadata(string key)
	{
		return metadata[key];
	}

	template<typename T>
	void Packet::removeMetadata(string key)
	{
		auto it = metadata.find(key);
		metadata.erase(it);
	}

	// Packet2

	Packet2::Packet2(shared_ptr<IConnection*> source, byteStream& stream)
		: Packet(source, stream)
	{
	}

	Packet2::~Packet2()
	{
	}
};
