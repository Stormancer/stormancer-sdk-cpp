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
		_dispatcher(config.dispatcher),
		_requestProcessor(_logger, vector<shared_ptr<IRequestModule>>()),
		_scenesDispatcher(),
		_metadata(config.metadata),
		_maxPeers(config.maxPeers),
		_systemSerializer(new MsgPackSerializer)
	{
		for (auto& serializer : config.serializers)
		{
			_serializers[serializer.get()->name] = serializer;
		}

		_metadata[L"serializers"] = Helpers::vectorJoin(Helpers::mapKeys(_serializers), L",");
		_metadata[L"transport"] = _transport.get()->name;
		_metadata[L"version"] = L"1.0.0";
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

	shared_ptr<ILogger> Client::logger()
	{
		return _logger;
	}

	void Client::setLogger(shared_ptr<ILogger> logger)
	{
		if (logger.get() != nullptr)
		{
			_logger = logger;
		}
		else
		{
			_logger = DefaultLogger::instance();
		}
	}

	task<shared_ptr<Scene>> Client::getPublicScene(wstring sceneId, wstring userData)
	{
		return _apiClient.getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([&](SceneEndpoint& sep) {
			return getScene(sceneId, sep);
		});
	}

	task<shared_ptr<Scene>> Client::getScene(wstring sceneId, SceneEndpoint sep)
	{
		return Helpers::taskIf(_serverConnection == nullptr, [this, &sep]() {
			return Helpers::taskIf(!_transport.get()->isRunning, [this]() {
				_cts = cancellation_token_source();
				return _transport.get()->start(L"client", make_shared<IConnectionManager>(new ConnectionHandler()), _cts.get_token(), 0, (uint16)(_maxPeers + 1));
			}).then([this, &sep]() {
				wstring endpoint = sep.tokenData.Endpoints[_transport.get()->name];
				return _transport.get()->connect(endpoint).then([this](shared_ptr<IConnection> connection) {
					_serverConnection = connection;

					for (auto& it : _metadata)
					{
						_serverConnection.get()->metadata[it.first] = it.second;
					}
				});
			});
		}).then([this, &sep]() {
				SceneInfosRequestDto parameter;
				parameter.Metadata = _serverConnection.get()->metadata;
				parameter.Token = sep.token;

				return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)MessageIDTypes::ID_GET_SCENE_INFOS, parameter);
			}).then([this, &sep, sceneId](SceneInfosDto& result) {
				if (!_serverConnection.get()->getComponent<MsgPackSerializer>())
				{
					if (result.SelectedSerializer.size() == 0)
					{
						throw string("No serializer selected.");
					}
					_serverConnection.get()->registerComponent(&_serializers[result.SelectedSerializer]);
					_serverConnection.get()->metadata[L"serializer"] = result.SelectedSerializer;
				}
				return make_shared<Scene>(new Scene(_serverConnection, this, sceneId, sep.token, result));
			});
	}

	task<void> Client::connectToScene(shared_ptr<Scene> scene, wstring& token, vector<Route&> localRoutes)
	{
		ConnectToSceneMsg parameter;
		parameter.Token = token;
		for (auto& r : localRoutes)
		{
			RouteDto routeDto;
			routeDto.Handle = r.index;
			routeDto.Metadata = r.metadata;
			routeDto.Name = r.name;
			parameter.Routes << routeDto;
		}
		parameter.ConnectionMetadata = _serverConnection.get()->metadata;

		return Client::sendSystemRequest<ConnectToSceneMsg, ConnectionResult>((byte)MessageIDTypes::ID_CONNECT_TO_SCENE, parameter)
			.then([&](ConnectionResult result) {
			scene.get()->completeConnectionInitialization(result);
			_scenesDispatcher.addScene(scene);
		});
	}

	void Client::disconnect()
	{
		if (_serverConnection != nullptr)
		{
			_serverConnection.get()->close();
		}
	}

	pplx::task<void> Client::disconnect(shared_ptr<Scene> scene, byte sceneHandle)
	{
		auto& _scenesDispatcher = this->_scenesDispatcher;
		return sendSystemRequest<byte, EmptyDto>((byte)MessageIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle)
			.then([this, &_scenesDispatcher, &sceneHandle](task<EmptyDto>& t) {
			_scenesDispatcher.removeScene(sceneHandle);
		});
	}

	template<typename T, typename U>
	task<U> Client::sendSystemRequest(byte id, T parameter)
	{
		return _requestProcessor.sendSystemRequest(_serverConnection, id, [this](byteStream& stream) {
			_systemSerializer.serialize(parameter, stream);
		}).then([this](Packet<> packet) {
			U res;
			_systemSerializer.deserialize(stream, res);
			return res;
		});
	}
};
