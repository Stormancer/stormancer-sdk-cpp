#include "stormancer.h"

namespace Stormancer
{
	// ConnectionHandler

	Client::ConnectionHandler::ConnectionHandler()
	{
		IConnectionManager::connectionCount = 0;
	}

	Client::ConnectionHandler::~ConnectionHandler()
	{
	}

	uint64 Client::ConnectionHandler::generateNewConnectionId()
	{
		return _current++;
	}

	void Client::ConnectionHandler::newConnection(IConnection* connection)
	{
	}

	IConnection* Client::ConnectionHandler::getConnection(uint64 id)
	{
		throw exception("Client::ConnectionHandler::getConnection not implemented.");
	}

	void Client::ConnectionHandler::closeConnection(IConnection* connection, wstring reason)
	{
	}


	// Client

	Client::Client(Configuration* config)
		: _initialized(false),
		_logger(ILogger::instance()),
		_accountId(config->account),
		_applicationName(config->application),
		_tokenHandler(new TokenHandler()),
		_apiClient(new ApiClient(config, _tokenHandler)),
		_transport(config->transport),
		_dispatcher(config->dispatcher),
		_requestProcessor(new RequestProcessor(_logger, vector<IRequestModule*>())),
		_scenesDispatcher(new SceneDispatcher),
		_metadata(config->metadata),
		_maxPeers(config->maxPeers)
	{
		_dispatcher->addProcessor(_requestProcessor);
		_dispatcher->addProcessor(_scenesDispatcher);

		_metadata[L"serializers"] = L"msgpack/map";
		_metadata[L"transport"] = _transport->name();
		_metadata[L"version"] = L"1.0.0";
		_metadata[L"platform"] = L"cpp";

		initialize();
	}

	Client::~Client()
	{
		disconnect();

		_cts.cancel();
		delete _scenesDispatcher;
		delete _requestProcessor;
		delete _apiClient;
		delete _tokenHandler;

		if (_serverConnection)
		{
			delete _serverConnection;
		}

		if (_dispatcher)
		{
			delete _dispatcher;
		}

		if (_transport)
		{
			delete _transport;
		}
	}

	void Client::initialize()
	{
		if (!_initialized)
		{
			_initialized = true;
			auto d2 = this->_dispatcher;
			_transport->packetReceived += new function<void(shared_ptr<Packet<>>)>(([this](shared_ptr<Packet<>> packet) {
				this->transport_packetReceived(packet);
			}));
		}
	}

	wstring Client::applicationName()
	{
		return _applicationName;
	}

	ILogger* Client::logger()
	{
		return _logger;
	}

	void Client::setLogger(ILogger* logger)
	{
		if (logger != nullptr)
		{
			_logger = logger;
		}
		else
		{
			_logger = ILogger::instance();
		}
	}

	pplx::task<shared_ptr<Scene>> Client::getPublicScene(wstring sceneId, wstring userData)
	{
		_logger->log(LogLevel::Trace, L"", L"Client::getPublicScene", sceneId);

		return _apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([this, sceneId](SceneEndpoint& sep) {
			return this->getScene(sceneId, sep);
		});
	}

	pplx::task<shared_ptr<Scene>> Client::getScene(wstring sceneId, SceneEndpoint sep)
	{
		_logger->log(LogLevel::Trace, L"", L"Client::getScene", sceneId);

		return Helpers::taskIf(_serverConnection == nullptr, [this, sep]() {
			return Helpers::taskIf(!_transport->isRunning(), [this]() {
				_cts = pplx::cancellation_token_source();
				return _transport->start(L"client", new ConnectionHandler(), _cts.get_token(), 11, (uint16)(_maxPeers + 1));
			}).then([this, sep]() {
				wstring endpoint = sep.tokenData.Endpoints.at(_transport->name());
				return _transport->connect(endpoint).then([this](IConnection* connection) {
					_serverConnection = connection;

					for (auto& it : this->_metadata)
					{
						_serverConnection->metadata[it.first] = it.second;
					}
				});
			});
		}).then([this, sep]() {
			SceneInfosRequestDto parameter;
			parameter.Metadata = _serverConnection->metadata;
			parameter.Token = sep.token;

			return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)MessageIDTypes::ID_GET_SCENE_INFOS, parameter);
		}).then([this, sep, sceneId](SceneInfosDto result) {
			this->_serverConnection->metadata[L"serializer"] = result.SelectedSerializer;
			return shared_ptr<Scene>( new Scene(_serverConnection, this, sceneId, sep.token, result) );
		});
	}

	pplx::task<void> Client::connectToScene(Scene* scene, wstring& token, vector<Route*> localRoutes)
	{
		ConnectToSceneMsg parameter;
		parameter.Token = token;
		for (auto r : localRoutes)
		{
			RouteDto routeDto;
			routeDto.Handle = r->_handle;
			routeDto.Metadata = r->metadata();
			routeDto.Name = r->name();
			parameter.Routes << routeDto;
		}
		parameter.ConnectionMetadata = _serverConnection->metadata;

		return Client::sendSystemRequest<ConnectToSceneMsg, ConnectionResult>((byte)MessageIDTypes::ID_CONNECT_TO_SCENE, parameter).then([this, scene](ConnectionResult result) {
			scene->completeConnectionInitialization(result);
			this->_scenesDispatcher->addScene(scene);
		});
	}

	void Client::disconnect()
	{
		disconnectAllScenes();

		if (_serverConnection != nullptr)
		{
			_serverConnection->close();
		}
	}

	pplx::task<void> Client::disconnect(Scene* scene, byte sceneHandle)
	{
		auto _scenesDispatcher = this->_scenesDispatcher;
		return sendSystemRequest<byte, EmptyDto>((byte)MessageIDTypes::ID_DISCONNECT_FROM_SCENE, sceneHandle).then([this, _scenesDispatcher, sceneHandle, scene](pplx::task<EmptyDto> t) {
			_scenesDispatcher->removeScene(sceneHandle);
		});
	}

	void Client::disconnectAllScenes()
	{
		for (auto scene : _scenesDispatcher->_scenes)
		{
			if (scene)
			{
				scene->disconnect();
			}
		}
	}

	void Client::transport_packetReceived(shared_ptr<Packet<>> packet)
	{
		// TODO plugins

		this->_dispatcher->dispatchPacket(packet);
	}
};
