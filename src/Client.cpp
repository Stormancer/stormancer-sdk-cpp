#include "libs.h"
#include "Client.h"
#include "Helpers.h"
#include "Infrastructure/ITokenHandler.h"
#include "Infrastructure/TokenHandler.h"

namespace Stormancer
{
	// ConnectionHandler

	uint64 Client::ConnectionHandler::generateNewConnectionId()
	{
		// TODO
		return _current++;
	}

	void Client::ConnectionHandler::newConnection(shared_ptr<IConnection*> connection)
	{
		throw new exception("Not Implemented");
	}

	shared_ptr<IConnection*> Client::ConnectionHandler::getConnection(uint64 id)
	{
		throw new exception("Not Implemented");
	}

	void Client::ConnectionHandler::closeConnection(shared_ptr<IConnection*> connection, string reason)
	{
		throw new exception("Not Implemented");
	}


	// Client

	Client::Client(ClientConfiguration& config)
		: _accountId(config.account),
		_applicationName(config.application),
		_tokenHandler(make_shared<ITokenHandler*>(new TokenHandler())),
		_apiClient(ApiClient(config, _tokenHandler)),
		_transport(config.transport),
		_dispatcher(config.dispatcher),
		_requestProcessor(RequestProcessor(_logger, list<IRequestModule>())),
		_scenesDispatcher(SceneDispatcher()),
		_metadata(config.metadata),
		_maxPeers(config.maxPeers)
	{
		for each (ISerializer& serializer in config.serializers)
		{
			_serializers.insert(serializer.name, serializer);
		}

		_metadata["serializers"] = Helpers::vectorJoin(Helpers::mapKeys(_serializers), ",");
		_metadata["transport"] = _transport.name;
		_metadata["version"] = "1.0.0a";
		_metadata["platform"] = "cpp";

		initialize();
	}

	Client::~Client()
	{
	}

	void Client::initialize()
	{

	}

	string Client::applicationName()
	{
		return _applicationName;
	}
};
