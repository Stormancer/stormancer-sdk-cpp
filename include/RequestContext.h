#pragma once
#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	class RequestContext
	{
	public:
		RequestContext(shared_ptr<Packet<>> packet);
		virtual ~RequestContext();

	public:
		shared_ptr<Packet<>> packet();
		bytestream* inputStream();
		bool isComplete();
		void send(function<void(bytestream*)> writer);
		void complete();
		void error(function<void(bytestream*)> writer);

	private:
		byte _requestId[2];
		shared_ptr<Packet<>> _packet = nullptr;
		bytestream* _stream = nullptr;
		bool _didSendValues = false;
		bool _isComplete;
	};
};
