#pragma once
#include "headers.h"
#include "ISerializable.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	/// Informations about a remote scene.
	struct SceneInfosDto : public ISerializable
	{
	public:
	
		/// Constructor.
		SceneInfosDto();
		
		/*! Constructor.
		\param stream The stream to deserialize.
		*/
		SceneInfosDto(bytestream* stream);
		
		/// Destructor.
		virtual ~SceneInfosDto();

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
		
		/// The scene id.
		wstring SceneId;
		
		/// The scene metadatas.
		stringMap Metadata;
		
		/// Vector of routes declared on the scene host.
		vector<RouteDto> Routes;
		
		/// The serializer the client should use when communicating with the scene.
		wstring SelectedSerializer;
	};
};
