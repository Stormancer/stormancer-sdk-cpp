#pragma once
#include "headers.h"
#include "Core/IConnection.h"

namespace Stormancer
{
	template<typename T = IConnection>
	class Packet
	{
	public:
		Packet(shared_ptr<T*> source, byteStream& stream, StringMap metadata = AnyMap());
		virtual ~Packet();

		template<typename TData>
		void setMetadata(string key, TData* data);

		template<typename TData>
		TData* getMetadata(string key);

		void removeMetadata(string key);

	public:
		byteStream& stream;
		shared_ptr<T*> connection;

	private:
		AnyMap metadata;
	};

	//class Packet2 : public Packet < IConnection >
	//{
	//public:
	//	Packet2(shared_ptr<IConnection*> source, byteStream& stream);
	//};
};
