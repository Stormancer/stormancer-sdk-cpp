#pragma once
#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	class RequestContext
	{
	public:
		RequestContext(Packet<>* packet);
		virtual ~RequestContext();

	public:
		Packet<>* packet();
		bytestream* inputStream();
		bool isComplete();
		void send(function<void(bytestream*)> writer);
		void complete();
		void error(function<void(bytestream*)> writer);

	private:
		byte _requestId[2];
		Packet<>* _packet = nullptr;
		bytestream* _stream = nullptr;
		bool _didSendValues = false;
		bool _isComplete;
	};
};
