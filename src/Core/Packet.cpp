
#include "Core/Packet.h"

namespace Stormancer
{
	// Packet

	template<typename T>
	Packet::Packet(shared_ptr<T*> source, byteStream& stream, anyMap metadata)
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
	anyMap Packet::metadata()
	{
		return _metadata;
	}

	template<typename T>
	template<typename TData>
	void Packet::setMetadata(string key, TData* data)
	{
		_metadata[key] = data;
	}

	template<typename T>
	template<typename TData>
	TData* Packet::getMetadata(string key)
	{
		return _metadata[key];
	}

	template<typename T>
	void Packet::removeMetadata(string key)
	{
		auto it = _metadata.find(key);
		_metadata.erase(it);
	}
};
