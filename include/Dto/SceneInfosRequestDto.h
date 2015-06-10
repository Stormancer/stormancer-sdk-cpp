#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	/// Informations about the requested scene.
	class SceneInfosRequestDto : public ISerializable
	{
	public:
	
		/// Constructor.
		SceneInfosRequestDto();
		
		/*! Constructor.
		\param stream The stream to deserialize.
		*/
		SceneInfosRequestDto(bytestream* stream);
		
		/// Destructor.
		virtual ~SceneInfosRequestDto();

	public:
	
		/*! The method to serialize the object.
		\param stream The stream where we serialize the object.
		*/
		void serialize(bytestream* stream);
		
		/*! The method to deserialize the object.
		\param stream The stream to deserialize.
		*/
		void deserialize(bytestream* stream);

	public:
	
		/// Authentication token containing informations about the target scene.
		wstring Token;
		
		/// Connexion metadatas.
		stringMap Metadata;
	};
};
