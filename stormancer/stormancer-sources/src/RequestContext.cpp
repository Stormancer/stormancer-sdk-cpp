#include "stormancer/stdafx.h"
#include "stormancer/RequestContext.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	RequestContext::RequestContext(Packet_ptr packet)
		: _packet(packet)
		, _stream()
		, _isComplete(false)
	{
		packet->stream >> _requestId;
		_stream.rdbuf()->pubsetbuf(reinterpret_cast<char*>(packet->stream.currentPtr()), packet->stream.rdbuf()->in_avail());
	}

	RequestContext::~RequestContext()
	{
	}

	Packet_ptr RequestContext::packet()
	{
		return _packet;
	}

	ibytestream& RequestContext::inputStream()
	{
		return _stream;
	}

	bool RequestContext::isComplete()
	{
		return _isComplete;
	}

	void RequestContext::send(const StreamWriter& streamWriter)
	{
		if (_isComplete)
		{
			throw std::runtime_error("The request is already completed.");
		}
		_didSendValues = true;
		auto requestId = _requestId;
		_packet->connection->send([requestId, &streamWriter](obytestream& stream) {
			stream << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG;
			stream << requestId;
			if (streamWriter)
			{
				streamWriter(stream);
			}
		}, 0);
	}

	void RequestContext::complete()
	{
		_isComplete = true;
		auto requestId = _requestId;
		auto didSendValues = _didSendValues;
		_packet->connection->send([requestId, didSendValues](obytestream& stream) {
			stream << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE;
			stream << requestId;
			byte c = (didSendValues ? 1 : 0);
			stream.write(&c, 1);
		}, 0);
	}

	void RequestContext::error(const StreamWriter& streamWriter)
	{
		auto requestId = _requestId;
		_packet->connection->send([requestId, &streamWriter](obytestream& stream) {
			stream << (byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR;
			stream << requestId;
			if (streamWriter)
			{
				streamWriter(stream);
			}
		}, 0);
	}
}
