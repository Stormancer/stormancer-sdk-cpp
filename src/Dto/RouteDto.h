#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Information about a scene route.
	struct RouteDto
	{
	public:

		/// Constructor.
		RouteDto();

		/// Destructor.
		virtual ~RouteDto();

		/// MessagePack serialization.
		template<typename Packer>
		void msgpack_pack(Packer& pk) const
		{
			pk.pack_map(3);

			pk.pack("Name");
			pk.pack(Name);

			pk.pack("Handle");
			pk.pack(Handle);

			pk.pack("Metadata");
			pk.pack(Metadata);
		}

		/// MessagePack deserialization.
		void msgpack_unpack(msgpack::object const& o);

	public:
		std::string Name;
		uint16 Handle;
		stringMap Metadata;
	};
};
