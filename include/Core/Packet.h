#pragma once
#include "headers.h"
#include "Core/IConnection.h"

namespace Stormancer
{
	template<typename T = IConnection>
	class Packet
	{
	public:
		Packet(shared_ptr<T*> source, byteStream& stream, anyMap metadata = anyMap());
		virtual ~Packet();

		anyMap metadata();

		template<typename TData>
		void setMetadata(string key, TData* data);

		template<typename TData>
		TData* getMetadata(string key);

		void removeMetadata(string key);

	public:
		byteStream& stream;
		shared_ptr<T*> connection;

	private:
		anyMap _metadata;
	};
};
