#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	/// Represents an empty network message
	struct EmptyDto : public ISerializable
	{
	public:
	
		/// Constructor.
		STORMANCER_DLL_API EmptyDto();
		
		/// Constructor.
		/// \param stream The stream to deserialize.
		STORMANCER_DLL_API EmptyDto(bytestream* stream);
		
		/// Destructor.
		STORMANCER_DLL_API virtual ~EmptyDto();

	public:
	
		/// The method to serialize the object.
		/// \param stream The stream where we serialize the object.
		void serialize(bytestream* stream);
		
		/// The method to deserialize the object.
		/// \param stream The stream to deserialize.
		void deserialize(bytestream* stream);
	};
};
