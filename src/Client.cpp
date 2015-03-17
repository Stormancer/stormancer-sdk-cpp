#include "Client.h"
#include "Helpers.h"

namespace Stormancer
{
	Client::Client(ClientConfiguration config)
		: _accountId(config.account),
		_applicationName(config.application),
		_tokenHandler(TokenHandler()),
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
};
