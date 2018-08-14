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
#include "stormancer/KeyStore.h"
#include "stormancer/AES/AESPacketTransform.h"
#include "stormancer/AES/IAES.h"
#include "stormancer/AES/AES.h"
#include "stormancer/PacketTransformProcessor.h"
#include "stormancer/Configuration.h"

void Stormancer::ConfigureContainer(std::weak_ptr<DependencyResolver> wResolver, Configuration_ptr config)
{
	auto resolver = wResolver.lock();

	resolver->registerDependency<KeyStore>([](std::weak_ptr<DependencyResolver>) {
		return std::make_shared<KeyStore>();
	}, true);

	resolver->registerDependency<Configuration>(config);

	resolver->registerDependency<ILogger>(config->logger);

	resolver->registerDependency<ITokenHandler>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<TokenHandler>(resolver.lock()->resolve<ILogger>());
	});

	resolver->registerDependency<ApiClient>([=](std::weak_ptr<DependencyResolver> resolver) {
		auto tokenHandler = resolver.lock()->resolve<ITokenHandler>();
		return std::make_shared<ApiClient>(resolver.lock()->resolve<ILogger>(), config, tokenHandler);
	}, true);

	resolver->registerDependency<Serializer>([](std::weak_ptr<DependencyResolver>) {
		return std::make_shared<Serializer>();
	}, true);

	resolver->registerDependency<IScheduler>(config->scheduler);

	resolver->registerDependency(config->transportFactory, true);

	resolver->registerDependency<IActionDispatcher>(config->actionDispatcher);

	resolver->registerDependency<P2PPacketDispatcher>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<P2PPacketDispatcher>(resolver.lock()->resolve<P2PTunnels>(), resolver.lock()->resolve<IConnectionManager>(), resolver.lock()->resolve<ILogger>());
	}, true);

	resolver->registerDependency<SceneDispatcher>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<SceneDispatcher>(resolver.lock()->resolve<IActionDispatcher>());
	}, true);

	resolver->registerDependency<SyncClock>([=](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<SyncClock>(resolver, config->synchronisedClockInterval);
	}, true);

	resolver->registerDependency<RequestProcessor>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<RequestProcessor>(resolver.lock()->resolve<ILogger>());
	}, true);

	resolver->registerDependency<PacketTransformProcessor>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<PacketTransformProcessor>(resolver.lock()->resolve<AESPacketTransform>());
	},true);

	resolver->registerDependency<IPacketDispatcher>([=](std::weak_ptr<DependencyResolver> resolver) {
		auto dispatcher = config->dispatcher(resolver);
		dispatcher->addProcessor(resolver.lock()->resolve<RequestProcessor>());
		dispatcher->addProcessor(resolver.lock()->resolve<SceneDispatcher>());
		dispatcher->addProcessor(resolver.lock()->resolve<P2PPacketDispatcher>());
		dispatcher->addProcessor(resolver.lock()->resolve<PacketTransformProcessor>());
		return dispatcher;
	}, true);
	
	resolver->registerDependency<AESPacketTransform>([](std::weak_ptr<DependencyResolver> resolver){
		return std::make_shared<AESPacketTransform>(resolver.lock()->resolve<IAES>(), resolver.lock()->resolve<Configuration>());
	}, false);

	resolver->registerDependency<IAES>([](std::weak_ptr<DependencyResolver> resolver) {


#if defined(_WIN32)
		return std::make_shared<AESWindows>(resolver.lock()->resolve<KeyStore>());






#endif

	}, true);

	resolver->registerDependency<IConnectionManager>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<ConnectionsRepository>(resolver.lock()->resolve<ILogger>());
	}, true);

	resolver->registerDependency<P2PSessions>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<P2PSessions>(resolver.lock()->resolve<IConnectionManager>());
	}, true);

	resolver->registerDependency<P2PTunnels>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<P2PTunnels>(
			resolver.lock()->resolve<RequestProcessor>(),
			resolver.lock()->resolve<IConnectionManager>(),
			resolver.lock()->resolve<Serializer>(),
			resolver.lock()->resolve<Configuration>(),
			resolver.lock()->resolve<ILogger>());
	}, true);

	resolver->registerDependency<P2PService>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<P2PService>(
			resolver.lock()->resolve<IConnectionManager>(),
			resolver.lock()->resolve<RequestProcessor>(),
			resolver.lock()->resolve<ITransport>(),
			resolver.lock()->resolve<Serializer>(),
			resolver.lock()->resolve<P2PTunnels>()
			);
	}, true);

	resolver->registerDependency<P2PRequestModule>([](std::weak_ptr<DependencyResolver> resolver) {
		return std::make_shared<P2PRequestModule>(
			resolver.lock()->resolve<ITransport>(),
			resolver.lock()->resolve<IConnectionManager>(),
			resolver.lock()->resolve<P2PSessions>(),
			resolver.lock()->resolve<Serializer>(),
			resolver.lock()->resolve<P2PTunnels>(),
			resolver.lock()->resolve<ILogger>(),
			resolver.lock()->resolve<Configuration>());
	}, true);
}
