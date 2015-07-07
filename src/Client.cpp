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
		throw std::runtime_error("Client::ConnectionHandler::getConnection not implemented.");
	}

	void Client::ConnectionHandler::closeConnection(IConnection* connection, std::string reason)
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
		_requestProcessor(new RequestProcessor(_logger, std::vector<IRequestModule*>())),
		_scenesDispatcher(new SceneDispatcher),
		_metadata(config->metadata),
		_maxPeers(config->maxPeers)
	{
		_dispatcher->addProcessor(_requestProcessor);
		_dispatcher->addProcessor(_scenesDispatcher);

		_metadata["serializers"] = "msgpack/array";
		_metadata["transport"] = _transport->name();
		_metadata["version"] = "1.0.0";
		_metadata["platform"] = "cpp";

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
			_transport->packetReceived += new std::function<void(std::shared_ptr<Packet<>>)>(([this](std::shared_ptr<Packet<>> packet) {
				this->transport_packetReceived(packet);
			}));
		}
	}

	std::string Client::applicationName()
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

	pplx::task<std::shared_ptr<Scene>> Client::getPublicScene(std::string sceneId, std::string userData)
	{
		_logger->log(LogLevel::Trace, "", "Client::getPublicScene", sceneId);

		return _apiClient->getSceneEndpoint(_accountId, _applicationName, sceneId, userData).then([this, sceneId](SceneEndpoint sep) {
			return this->getScene(sceneId, sep);
		});
	}

	pplx::task<std::shared_ptr<Scene>> Client::getScene(std::string sceneId, SceneEndpoint sep)
	{
		_logger->log(LogLevel::Trace, "", "Client::getScene", sceneId);

		return taskIf(_serverConnection == nullptr, [this, sep]() {
			return taskIf(!_transport->isRunning(), [this]() {
				_cts = pplx::cancellation_token_source();
				return _transport->start("client", new ConnectionHandler(), _cts.get_token(), 11, (uint16)(_maxPeers + 1));
			}).then([this, sep]() {
				std::string endpoint = sep.tokenData.Endpoints.at(_transport->name());
				return _transport->connect(endpoint).then([this](IConnection* connection) {
					_serverConnection = connection;
					_serverConnection->metadata = this->_metadata;
				});
			});
		}).then([this, sep]() {
			SceneInfosRequestDto parameter;
			parameter.Metadata = _serverConnection->metadata;
			parameter.Token = sep.token;
			return sendSystemRequest<SceneInfosRequestDto, SceneInfosDto>((byte)MessageIDTypes::ID_GET_SCENE_INFOS, parameter);
		}).then([this, sep, sceneId](SceneInfosDto result) {
			this->_serverConnection->metadata["serializer"] = result.SelectedSerializer;
			return std::shared_ptr<Scene>(new Scene(_serverConnection, this, sceneId, sep.token, result));
		});
	}

	pplx::task<void> Client::connectToScene(Scene* scene, std::string& token, std::vector<Route*> localRoutes)
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
			_serverConnection = nullptr;
		}
	}

	pplx::task<void> Client::disconnect(Scene* scene, byte sceneHandle)
	{
		auto _scenesDispatcher = this->_scenesDispatcher;
		DisconnectFromSceneDto dto(sceneHandle);
		return sendSystemRequest<DisconnectFromSceneDto, EmptyDto>((byte)MessageIDTypes::ID_DISCONNECT_FROM_SCENE, dto)
			.then([this, _scenesDispatcher, sceneHandle, scene](pplx::task<EmptyDto> t) {
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

	void Client::transport_packetReceived(std::shared_ptr<Packet<>> packet)
	{
		// TODO plugins

		this->_dispatcher->dispatchPacket(packet);
	}
};
