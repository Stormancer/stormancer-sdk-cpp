#include "stormancer.h"

namespace Stormancer
{
	RequestContext::RequestContext(Packet<>* packet)
		: _packet(packet),
		_stream(packet->stream)
	{
		(*packet->stream) >> _requestId[0];
		(*packet->stream) >> _requestId[1];

		Helpers::streamCopy(packet->stream, _stream);
	}

	RequestContext::~RequestContext()
	{
	}

	Packet<>* RequestContext::packet()
	{
		return _packet;
	}

	bytestream* RequestContext::inputStream()
	{
		return _stream;
	}

	bool RequestContext::isComplete()
	{
		return _isComplete;
	}

	void RequestContext::send(function<void(bytestream*)> writer)
	{
		if (_isComplete)
		{
			throw exception("The request is already completed.");
		}
		_didSendValues = true;
		_packet->connection->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, [this, &writer](bytestream* stream) {
			stream->write((char*)this->_requestId, 2);
			writer(stream);
		});
	}

	void RequestContext::complete()
	{
		_packet->connection->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, [this](bytestream* stream) {
			stream->write((char*)_requestId, 2);
			char c = (this->_didSendValues ? 1 : 0);
			stream->write(&c, 1);
		});
	}

	void RequestContext::error(function<void(bytestream*)> writer)
	{
		_packet->connection->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, [this, &writer](bytestream* stream) {
			stream->write((char*)_requestId, 2);
			writer(stream);
		});
	}
};
