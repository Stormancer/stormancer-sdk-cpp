#pragma once
#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	/// System request context
	class RequestContext
	{
	public:
	
		/*! Constructor
		\param packet The packet.
		*/
		RequestContext(shared_ptr<Packet<>> packet);
		
		/// Destructor.
		virtual ~RequestContext();

	public:
		shared_ptr<Packet<>> packet();
		bytestream* inputStream();
		
		/// Returns the request completed state.
		bool isComplete();
		
		/*! Sends a response to the request.
		\param writer A method used to write in the request stream.
		*/
		void send(function<void(bytestream*)> writer);
		
		/// Completes the system request.
		void complete();
		
		/*! Completes the system request with an error.
		\param writer A method for writing the error.
		*/
		void error(function<void(bytestream*)> writer);

	private:
		byte _requestId[2];
		
		/// Packet that initiated the request
		shared_ptr<Packet<>> _packet = nullptr;
		
		/// Stream exposing the request parameters.
		bytestream* _stream = nullptr;
		
		bool _didSendValues = false;
		bool _isComplete;
	};
};
