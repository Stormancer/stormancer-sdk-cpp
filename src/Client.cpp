#include "stormancer.h"

namespace Stormancer
{
	// ConnectionHandler

	Client::ConnectionHandler::ConnectionHandler()
	{
		connectionCount = 0;
	}

	Client::ConnectionHandler::~ConnectionHandler()
	{
	}

	uint64 Client::ConnectionHandler::generateNewConnectionId()
	{
		// TODO
		return _current++;
	}

	void Client::ConnectionHandler::newConnection(shared_ptr<IConnection> connection)
	{
	}

	shared_ptr<IConnection> Client::ConnectionHandler::getConnection(uint64 id)
	{
		throw string("Not Implemented.");
	}

	void Client::ConnectionHandler::closeConnection(shared_ptr<IConnection> connection, wstring reason)
	{
	}


	// Client

	Client::Client(ClientConfiguration& config)
		: _accountId(config.account),
		_applicationName(config.application),
		_tokenHandler(new TokenHandler()),
		_apiClient(config, _tokenHandler),
		_serverConnection(nullptr),
		_transport(config.transport),
		//_dispatcher(config.dispatcher),
		//_requestProcessor(RequestProcessor(_logger, list<shared_ptr<IRequestModule*>>())),
		//_scenesDispatcher(SceneDispatcher()),
		_metadata(config.metadata),
		_maxPeers(config.maxPeers)
	{
		/*for each (ISerializer& serializer in config.serializers)
		{
			_serializers.insert(serializer.name, serializer);
		}*/

		//_metadata["serializers"] = Helpers::vectorJoin(Helpers::mapKeys(_serializers), ",");
		//_metadata["transport"] = _transport.name;
		_metadata[L"version"] = L"1.0.0a";
		_metadata[L"platform"] = L"cpp";

		initialize();
	}

	Client::~Client()
	{
	}

	void Client::initialize()
	{

	}

	wstring Client::applicationName()
	{
		return _applicationName;
	}

	/*shared_ptr<ILogger*> Client::logger()
	{
		return _logger;
	}

	void Client::setLogger(shared_ptr<ILogger*> logger)
	{
		if ((*logger) != nullptr)
		{
			_logger = logger;
		}
		else
		{
			_logger = DefaultLogger::instance();
		}
	}*/

	task<Scene> Client::getPublicScene(wstring sceneId, wstring userData)
	{
		return _apiClient.getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([&](SceneEndpoint& sep) {
			return getScene(sceneId, sep);
		});
	}

	task<Scene> Client::getScene(wstring sceneId, SceneEndpoint sep)
	{
		// TODO

		if (_serverConnection.get() == nullptr)
		{
			if (!_transport.get()->isRunning)
			{
				_cts = cancellation_token_source();
				_transport.get()->start(L"client", make_shared<IConnectionManager>(new ConnectionHandler()), _cts.get_token(), 0, (_maxPeers+1));
			}
			wstring endpoint = sep.tokenData.Endpoints[_transport.get()->name];
			_serverConnection = make_shared<IConnection>(_transport.get()->connect(endpoint));

			for (auto i = _metadata.begin(); i != _metadata.end(); i++)
			{
				_serverConnection.get()->metadata[i->first] = i->second;
			}
		}
		SceneInfosRequestDto parameter;
		parameter.Metadata = _serverConnection.get()->metadata;
		parameter.Token = sep.token;

		SceneInfosDto result = sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)MessageIDTypes.ID_GET_SCENE_INFOS, parameter);

		if (_serverConnection.get()->getComponent<shared_ptr<ISerializer>>().get() == nullptr)
		{
			if (result.SelectedSerializer.size() == 0)
			{
				throw string("No serializer selected.");
			}
			_serverConnection.get()->registerComponent(_serializers[result.SelectedSerializer]);
			_serverConnection.get()->metadata["serializer"] = result.SelectedSerializer;
		}
		Scene scene(_serverConnection, this, sceneId, sep.token, result);
		
		//_pluginCtx.sceneCreated(scene);
		return scene;
	}
};
