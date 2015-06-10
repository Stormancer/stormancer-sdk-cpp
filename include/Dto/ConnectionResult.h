#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	/// Result of a connection attempt.
	class ConnectionResult : public ISerializable
	{
	public:
	
		/// Constructor.
		ConnectionResult();
		
		/*! Constructor.
		\param stream The stream to deserialize.
		*/
		ConnectionResult(bytestream* stream);
		
		/// Destructor.
		virtual ~ConnectionResult();

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
		/// Handle of the scene the client was connected to.
		byte SceneHandle;
		
		/// Route mappings in the scene (ie : routeName => routeHandle)
		map<wstring, uint16> RouteMappings;
	};
};
