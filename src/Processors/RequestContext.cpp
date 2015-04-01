#include "stormancer.h"

namespace Stormancer
{
	RequestContext::RequestContext(Packet<>& packet)
		: _packet(packet)
	{
		packet.stream >> _requestId[0];
		packet.stream >> _requestId[1];

		Helpers::streamCopy(packet.stream, _stream);
	}

	RequestContext::~RequestContext()
	{
	}

	Packet<>& RequestContext::packet()
	{
		return _packet;
	}

	byteStream& RequestContext::inputStream()
	{
		return _stream;
	}

	bool RequestContext::isComplete()
	{
		return _isComplete;
	}

	void RequestContext::send(function<void(byteStream&)> writer)
	{
		if (_isComplete)
		{
			throw string("The request is already completed.");
		}
		_didSendValues = true;
		auto& _requestId = this->_requestId;
		_packet.connection.get()->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG, [this, &writer, &_requestId](byteStream& stream) {
			stream.write((char*)_requestId, 2);
			writer(stream);
		});
	}

	void RequestContext::complete()
	{
		auto& didSendValues = _didSendValues;
		auto& _requestId = this->_requestId;
		_packet.connection.get()->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE, [this, &didSendValues, &_requestId](byteStream& stream) {
			stream.write((char*)_requestId, 2);
			char c = (didSendValues ? 1 : 0);
			stream.write(&c, 1);
		});
	}

	void RequestContext::error(function<void(byteStream&)> writer)
	{
		auto& _requestId = this->_requestId;
		_packet.connection.get()->sendSystem((byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR, [this, &writer, &_requestId](byteStream& stream) {
			stream.write((char*)_requestId, 2);
			writer(stream);
		});
	}
};
