#pragma once

#include "stormancer/BuildConfig.h"



namespace Stormancer
{
	/// Informations about the requested scene.
	class SceneInfosRequestDto
	{
	public:
	
		/// Authentication token containing informations about the target scene.
		std::string Token;
		
		/// Connexion metadatas.
		std::map<std::string, std::string> Metadata;

		MSGPACK_DEFINE_MAP(Token, Metadata);
	};
};
