#pragma once

#include "headers.h"

namespace Stormancer
{
	/// Informations about the requested scene.
	class SceneInfosRequestDto
	{
	public:

		/// Constructor.
		SceneInfosRequestDto();

		/// Destructor.
		virtual ~SceneInfosRequestDto();

		/// MessagePack serialization.
		template<typename Packer>
		void msgpack_pack(Packer& pk) const
		{
			pk.pack_map(2);

			pk.pack("Token");
			pk.pack(Token);

			pk.pack("Metadata");
			pk.pack(Metadata);
		}

	public:
	
		/// Authentication token containing informations about the target scene.
		std::string Token;
		
		/// Connexion metadatas.
		std::map<std::string, std::string> Metadata;
	};
};
