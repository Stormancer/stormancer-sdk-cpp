#pragma once
#include "headers.h"
#include "ISerializable.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	/// Required parameters to connect to a scene.
	class ConnectToSceneMsg : public ISerializable
	{
	public:
	
		/// Constructor.
		ConnectToSceneMsg();
		
		/*! Constructor
		\param stream The stream to deserialize.
		*/
		ConnectToSceneMsg(bytestream* stream);
		
		/// Destructor.
		virtual ~ConnectToSceneMsg();

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
	
		/// Authentication token.
		wstring Token;
		
		/// List of client routes.
		vector<RouteDto> Routes;
		
		/// Connection Metadatas.
		stringMap ConnectionMetadata;
	};
};
