#include "stormancer/stdafx.h"
#include "stormancer/RequestContext.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	RequestContext::RequestContext(Packet_ptr packet)
		: _packet(packet)
		, _stream(new ibytestream())
		, _isComplete(false)
	{
		(*packet->stream) >> _requestId;
		_stream->rdbuf()->pubsetbuf(packet->stream->currentPtr(), packet->stream->rdbuf()->in_avail());
	}

	RequestContext::~RequestContext()
	{
		delete _stream;
	}

	Packet_ptr RequestContext::packet()
	{
		return _packet;
	}

	ibytestream* RequestContext::inputStream()
	{
		return _stream;
	}

	bool RequestContext::isComplete()
	{
		return _isComplete;
	}

	void RequestContext::send(const Writer& writer)
	{
		if (_isComplete)
		{
			throw std::runtime_error("The request is already completed.");
		}
		_didSendValues = true;
		_packet->connection->send([=, &writer](obytestream* stream) {
			(*stream) << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG;
			(*stream) << _requestId;
			if (writer)
			{
				writer(stream);
			}
		}, 0);
	}

	void RequestContext::complete()
	{
		_isComplete = true;
		_packet->connection->send([=](obytestream* stream) {
			(*stream) << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE;
			(*stream) << _requestId;
			byte c = (_didSendValues ? 1 : 0);
			stream->write(&c, 1);
		}, 0);
	}

	void RequestContext::error(const Writer& writer)
	{
		_packet->connection->send([=, &writer](obytestream* stream) {
			(*stream) << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR;
			(*stream) << _requestId;
			if (writer)
			{
				writer(stream);
			}
		}, 0);
	}
};
