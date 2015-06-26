#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Information about a scene route.
	struct DisconnectFromSceneDto
	{
	public:

		/// Constructor.
		DisconnectFromSceneDto(byte sceneHandle = 0);

		/// Destructor.
		virtual ~DisconnectFromSceneDto();

		/// MessagePack serialization.
		template<typename Packer>
		void msgpack_pack(Packer& pk) const
		{
			pk.pack_map(SceneHandle); // serialize byte
		}

	public:
		byte SceneHandle;
	};
};
