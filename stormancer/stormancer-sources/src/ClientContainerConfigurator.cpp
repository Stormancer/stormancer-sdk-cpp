#include "stormancer/stdafx.h"
#include "stormancer/TokenHandler.h"
#include "stormancer/ApiClient.h"
#include "stormancer/Serializer.h"
#include "stormancer/IScheduler.h"
#include "stormancer/SceneDispatcher.h"
#include "stormancer/SyncClock.h"
#include "stormancer/P2P/P2PPacketDispatcher.h"
#include "stormancer/P2P/ConnectionsRepository.h"
#include "stormancer/P2P/P2PSessions.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/KeyStore.h"
#include "stormancer/AES/AESPacketTransform.h"
#include "stormancer/AES/IAES.h"
#include "stormancer/AES/AES.h"
#include "stormancer/PacketTransformProcessor.h"
#include "stormancer/Configuration.h"
#include "stormancer/Client.h"
#include "stormancer/IPacketDispatcher.h"

void Stormancer::Client::ConfigureContainer(ContainerBuilder& builder, Configuration_ptr config)
{
	std::weak_ptr<Stormancer::Client> weakThis = shared_from_this();

	builder.registerDependency<Client>([weakThis](const DependencyScope&) { return weakThis.lock(); }).as<IClient>().asSelf().instancePerRequest();

	builder.registerDependency<KeyStore>().singleInstance();

	builder.registerDependency<Configuration>(config);

	builder.registerDependency<ILogger>(config->logger);

	builder.registerDependency<TokenHandler, ILogger>().as<ITokenHandler>();

	builder.registerDependency<ApiClient, ILogger, Configuration, ITokenHandler>().singleInstance();

	builder.registerDependency<Serializer>().singleInstance();

	builder.registerDependency<IScheduler>(config->scheduler);

	builder.registerDependency<ITransport>(config->transportFactory).singleInstance();

	builder.registerDependency<IActionDispatcher>(config->actionDispatcher);

	builder.registerDependency<P2PPacketDispatcher, P2PTunnels, IConnectionManager, ILogger, Serializer>().singleInstance();

	builder.registerDependency<SceneDispatcher, IActionDispatcher,ILogger>().singleInstance();

	builder.registerDependency<SyncClock>([](const DependencyScope& scope)
	{
		return std::make_shared<SyncClock>(scope, scope.resolve<Configuration>()->synchronisedClockInterval);
	}).singleInstance();
	
	builder.registerDependency<RequestProcessor, ILogger>().singleInstance();

	builder.registerDependency<PacketTransformProcessor, AESPacketTransform>().singleInstance();

	builder.registerDependency<IPacketDispatcher>([](const DependencyScope& scope)
	{
		auto dispatcher = scope.resolve<Configuration>()->dispatcher(scope);
		dispatcher->addProcessor(scope.resolve<RequestProcessor>());
		dispatcher->addProcessor(scope.resolve<SceneDispatcher>());
		dispatcher->addProcessor(scope.resolve<P2PPacketDispatcher>());
		dispatcher->addProcessor(scope.resolve<PacketTransformProcessor>());
		return dispatcher;
	}).singleInstance();
	
	builder.registerDependency<AESPacketTransform, IAES, Configuration>();

	builder.registerDependency<IAES>([](const DependencyScope& scope)
	{


#if defined(_WIN32)
		return std::make_shared<AESWindows>(scope.resolve<KeyStore>());






#else
#include "stormancer/Linux/AES/AES_Linux.h"
		return std::make_shared<AESLinux>(scope.resolve<KeyStore>());
#endif
	}).singleInstance();

	builder.registerDependency<ConnectionsRepository, ILogger>().as<IConnectionManager>().singleInstance();

	builder.registerDependency<P2PSessions, IConnectionManager>().singleInstance();

	builder.registerDependency<P2PTunnels, RequestProcessor, IConnectionManager, Serializer, Configuration, ILogger>().singleInstance();

	builder.registerDependency<P2PService, IConnectionManager, RequestProcessor, ITransport, Serializer, P2PTunnels>().singleInstance();

	builder.registerDependency<P2PRequestModule, ITransport, IConnectionManager, P2PSessions, Serializer, P2PTunnels, ILogger, Configuration, Client>().singleInstance();
}
