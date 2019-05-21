#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Packet.h"

namespace Stormancer
{
	/// System request context
	class RequestContext
	{
	public:
	
		/*! Constructor
		\param packet The packet.
		*/
		RequestContext(Packet_ptr packet);
		
		/// Destructor.
		virtual ~RequestContext();

		Packet_ptr packet();

		ibytestream& inputStream();
		
		/// Returns the request completed state.
		bool isComplete();
		
		/*! Sends a response to the request.
		\param streamWriter A method used to write in the request stream.
		*/
		void send(const StreamWriter& streamWriter);
		
		/// Completes the system request.
		void complete();
		
		/*! Completes the system request with an error.
		\param streamWriter A method for writing the error.
		*/
		void error(const StreamWriter& streamWriter);

	private:

		uint16 _requestId;
		
		/// Packet that initiated the request
		Packet_ptr _packet = nullptr;
		
		/// Stream exposing the request parameters.
		ibytestream _stream;
		
		bool _didSendValues = false;

		bool _isComplete = false;
	};
}
