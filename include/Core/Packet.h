#pragma once
#include "stdafx.h"
#include "Core/IConnection.h"

namespace Stormancer
{
	template<typename T>
	class Packet
	{
	public:
		Packet(T& source, stringbuf& stream, StringMap metadata = AnyMap());
		virtual ~Packet();

		template<typename TData>
		void setMetadata(string key, TData* data);

		template<typename TData>
		TData* getMetadata(string key);

		void removeMetadata(string key);

	public:
		stringbuf& stream;
		T& connection;

	private:
		AnyMap metadata;
	};

	class Packet2 : public Packet < IConnection >
	{
	public:
		Packet2(IConnection& source, stringbuf& stream);
	};
};
