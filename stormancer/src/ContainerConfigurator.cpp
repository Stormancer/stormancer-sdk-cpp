#include "stormancer/stdafx.h"
#include "stormancer/ContainerConfigurator.h"
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

void Stormancer::ConfigureContainer(DependencyResolver* dr, Configuration_ptr config)
{
	dr->registerDependency<Configuration>(config);
	dr->registerDependency<ILogger>(config->logger);

	dr->registerDependency<ITokenHandler>([](DependencyResolver* dr) {
		return std::make_shared<TokenHandler>(dr->resolve<ILogger>());
	});

	dr->registerDependency<ApiClient>([=](DependencyResolver* resolver) {
		auto tokenHandler = resolver->resolve<ITokenHandler>();
		return std::make_shared<ApiClient>(resolver->resolve<ILogger>(), config, tokenHandler);
	}, true);

	dr->registerDependency<Serializer>([](DependencyResolver*) {
		return std::make_shared<Serializer>();
	}, true);

	dr->registerDependency<IScheduler>(config->scheduler);

	dr->registerDependency(config->transportFactory, true);

	dr->registerDependency<IActionDispatcher>(config->actionDispatcher);


	dr->registerDependency<P2PPacketDispatcher>([](DependencyResolver* dr) {
		return std::make_shared<P2PPacketDispatcher>(dr->resolve<P2PTunnels>(), dr->resolve<IConnectionManager>(), dr->resolve<ILogger>());
	}, true);
	dr->registerDependency<SceneDispatcher>([](DependencyResolver* resolver) {
		return std::make_shared<SceneDispatcher>(resolver->resolve<IActionDispatcher>());
	}, true);

	dr->registerDependency<SyncClock>([=](DependencyResolver* resolver) {
		return std::make_shared<SyncClock>(resolver, config->synchronisedClockInterval);
	}, true);

	dr->registerDependency<RequestProcessor>([](DependencyResolver* resolver) {
		return std::make_shared<RequestProcessor>(resolver->resolve<ILogger>());
	}, true);

	dr->registerDependency<IPacketDispatcher>([=](DependencyResolver* dependencyResolver) {
		auto dispatcher = config->dispatcher(dependencyResolver);
		dispatcher->addProcessor(dependencyResolver->resolve<RequestProcessor>());
		dispatcher->addProcessor(dependencyResolver->resolve<SceneDispatcher>());
		dispatcher->addProcessor(dependencyResolver->resolve<P2PPacketDispatcher>());
		return dispatcher;
	}, true);

	dr->registerDependency<IConnectionManager>([](DependencyResolver* dr) {
		return std::make_shared<ConnectionsRepository>(dr->resolve<ILogger>());
	}, true);

	dr->registerDependency<P2PSessions>([](DependencyResolver* dr) {
		return std::make_shared<P2PSessions>(dr->resolve<IConnectionManager>());
	}, true);

	dr->registerDependency<P2PTunnels>([](DependencyResolver* dr) {

		return std::make_shared<P2PTunnels>(
			dr->resolve<RequestProcessor>(),
			dr->resolve<IConnectionManager>(),
			dr->resolve<Serializer>(), 
			dr->resolve<Configuration>(),
			dr->resolve<ILogger>());
	}, true);

	dr->registerDependency<P2PService>([](DependencyResolver* dr) {


		return std::make_shared<P2PService>(
			dr->resolve<IConnectionManager>(),
			dr->resolve<RequestProcessor>(),
			dr->resolve<ITransport>(),
			dr->resolve<Serializer>(),
			dr->resolve<P2PTunnels>()
			);
	}, true);

	dr->registerDependency<P2PRequestModule>([](DependencyResolver* dr) {

		return std::make_shared<P2PRequestModule>(
			dr->resolve<ITransport>(),
			dr->resolve<IConnectionManager>(),
			dr->resolve<P2PSessions>(),
			dr->resolve<Serializer>(),
			dr->resolve<P2PTunnels>(),
			dr->resolve<ILogger>(),
			dr->resolve<Configuration>());
	}, true);
}
