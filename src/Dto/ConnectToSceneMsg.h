#pragma once
#include "headers.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	/// Required parameters to connect to a scene.
	class ConnectToSceneMsg
	{
	public:

		/// Constructor.
		ConnectToSceneMsg();

		/// Destructor.
		virtual ~ConnectToSceneMsg();

		/// MessagePack serialization.
		template<typename Packer>
		void msgpack_pack(Packer& pk) const
		{
			pk.pack_map(4);

			pk.pack("Token");
			pk.pack(Token);

			pk.pack("Routes");
			pk.pack(Routes);

			pk.pack("SceneMetadata");
			pk.pack(SceneMetadata);

			pk.pack("ConnectionMetadata");
			pk.pack(ConnectionMetadata);
		}

	public:

		/// Authentication token.
		std::string Token;

		/// List of client routes.
		std::vector<RouteDto> Routes;

		/// Scene Metadatas.
		stringMap SceneMetadata;

		/// Connection Metadatas.
		stringMap ConnectionMetadata;
	};
};
