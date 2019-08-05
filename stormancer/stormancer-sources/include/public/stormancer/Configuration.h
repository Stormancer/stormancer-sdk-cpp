#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/IPlugin.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/Version.h"
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>
#include <functional>


namespace Stormancer
{
	enum class EndpointSelectionMode
	{
		FALLBACK = 0,
		RANDOM = 1
	};
	class ITransport;
	class IPacketDispatcher;
	class Configuration;
	using Configuration_ptr = std::shared_ptr<Configuration>;
	class DependencyResolver;
	class IActionDispatcher;
	class IScheduler;

	/// <summary>
	/// This class contains parameters needed to initialize and configure a <c>IClient</c>.
	/// </summary>
	/// <remarks>
	/// Creating a <c>Configuration</c> using <see cref="Configuration::create()"/> is the first step before creating a <c>IClient</c>
	/// to start using Stormancer functionality.
	/// </remarks>
	class STORMANCER_DLL_API Configuration
	{
	public:

		friend class Client;

#pragma region public_methods

		~Configuration();

		/// <summary>
		/// Factory method for <c>Configuration</c>.
		/// Always use this method to instantiate <c>Configuration</c>.
		/// </summary>
		/// <param name="endpoint">HTTP endpoint of the Stormancer cluster to connect to.
		/// Once the configuration is instantiated, you can add more candidate endpoints by calling <c>addServerEndpoint()</c>.</param>
		/// <param name="account">Stormancer account to use on the cluster.</param>
		/// <param name="application">Name of the Stormancer application to connect to. It must belong to the chosen account.</param>
		/// <returns>A <c>Configuration</c> <c>std::shared_ptr</c> with the supplied parameters.</returns>
		static std::shared_ptr<Configuration> create(const std::string& endpoint, const std::string& account, const std::string& application)
		{
			Version::checkVersionLinkTime(); /* Causes linker error in case of header mismatch if STORM_CHECK_VERSION_LINKTIME is defined */
			// CreateInternal is needed to keep the deleter of the shared_ptr inside the lib
			return createInternal(endpoint, account, application, Version::getHeadersVersionString());
		}

		/// <summary>
		/// Add a candidate server endpoint.
		/// </summary>
		/// <param name="serverEndpoint">HTTP endpoint to add to the list of candidates.</param>
		void addServerEndpoint(const std::string& serverEndpoint);

		/// <summary>
		/// Get the list of candidate server endpoints for this <c>Configuration</c>.
		/// </summary>
		/// <returns>List of candidate endpoints.</returns>
		std::vector<std::string> getApiEndpoint();

		/// <summary>
		/// Add a plugin to the client.
		/// </summary>
		/// <remarks>
		/// Plugins enable developers to plug custom code in the stormancer client's extensibility points. Possible uses include: custom high level protocols, logger or analyzers.
		/// </remarks>
		/// <param name="plugin">The plugin instance to add.
		/// It must be dynamically allocated by you, and will be freed by the <c>Configuration</c> when it is destroyed.</param>
		void addPlugin(IPlugin* plugin);

		/// <summary>
		/// Get the list of plugins in this <c>Configuration</c>.
		/// Use <c>addPlugin()</c> to add new plugins.
		/// </summary>
		/// <returns>The <c>Configuration</c>'s list of plugins.</returns>
		const std::vector<IPlugin*> plugins();

		/// <summary>
		/// Check whether this <c>Configuration</c> has a public IP address setup.
		/// </summary>
		/// <remarks>
		/// This is typically used for dedicated game servers.
		/// To set the public IP for the <c>Configuration</c>, use the <c>dedicatedServerEndpoint</c> member variable.
		/// </remarks>
		/// <returns><c>true</c> if this <c>Configuration</c> has a public IP setup.</returns>
		const bool hasPublicIp();

		/// <summary>
		/// Return the IP:Port couple setup for this <c>Configuration</c>.
		/// </summary>
		/// <returns>A <c>std::string</c> in the <c>ip:port</c> format.</returns>
		const std::string getIp_Port();
#pragma endregion

#pragma region public_members

		/// <summary>
		/// A string containing the account name of the application.
		/// </summary>
		const std::string account = "";

		/// <summary>
		/// A string containing the name of the application.
		/// </summary>
		const std::string application = "";

		

		/// <summary>
		/// Maximum number of remote peers that can connect with this client.
		/// </summary>
		uint16 maxPeers = 10;

		/// <summary>
		/// Port that this client's stormancer transport socket should bind to.
		/// By default, set to 0 for automatic attribution.
		/// </summary>
		uint16 clientSDKPort = 0;

		/// <summary>
		/// Was used to enable or disable asynchrounous dispatch of received messages.
		/// Has no effect.
		/// If you want to control how message handlers are dispatched, use <c>actionDispatcher</c> instead.
		/// </summary>
		/// \deprecated Set a custom <see cref="actionDispatcher"/> instead.
		bool asynchronousDispatch = true;

		/// <summary>
		/// On non-Win32 platforms, set the size of the threadpool (number of threads) for the default dispatcher.
		/// </summary>
		int threadpoolSize = 10;

		

		/// <summary>
		/// Local application port for direct communication with other clients, as P2P host or dedicated server.
		/// </summary>
		unsigned short serverGamePort = 7777;

		/// <summary>
		/// If this stormancer client runs on a dedicated server, set this to the public IP of the server.
		/// This enables other clients to connect to it directly.
		/// </summary>
		std::string dedicatedServerEndpoint;

		/// <summary>
		/// Disable or enable nat punch through on client side.
		/// </summary>
		bool enableNatPunchthrough = true;

		/// <summary>
		/// Force a specific endpoint. Configuration used to connect client directly by localhost address.
		/// </summary>
		std::string forceTransportEndpoint = "";

		/// <summary>
		/// Provide a custom <c>IActionDispatcher</c> to control how event handlers are dispatched.
		/// </summary>
		/// <remarks>
		/// By default, events are dispatched on the current network thread. Replace the dispatcher with a MainThreadActionDispatcher to dispatch to your main thread.
		/// your main game loop for instance.
		/// An event can be an incoming route message, an RPC response, a subscription notification, etc.
		/// Generally, it is anything in the Stormancer API that you can register a handler for,
		/// be it a <c>pplx::task</c> continuation, an <c>Action</c>, an <c>Event</c> or an <c>rxcpp::subscription</c>.
		/// </remarks>
		std::shared_ptr<IActionDispatcher> actionDispatcher;

		/// <summary>
		/// Whether the Stormancer's synchronized clock should be enabled on this client or not.
		/// </summary>
		bool synchronisedClock = true;

		/// <summary>
		/// The interval between successive ping requests for the synchronized clock, in milliseconds. Default is 5000 ms.
		/// </summary>
		int32 synchronisedClockInterval = 5000;

		/// <summary>
		/// Set how an endpoint should be selected among the candidates.
		/// </summary>
		EndpointSelectionMode endpointSelectionMode = EndpointSelectionMode::FALLBACK;

		/// <summary>
		/// Set the logger for this stormancer client.
		/// </summary>
		/// <remarks>
		/// By default, it will be <c>NullLogger</c>, which doesn't print anything.
		/// There are a number of predefined loggers that you can pick from, or implement your own.
		/// <see cref="ConsoleLogger"/>
		/// <see cref="FileLogger"/>
		/// <see cref="VisualStudioLogger"/>
		/// </remarks>
		ILogger_ptr logger;

		/// <summary>
		/// The default timeout duration for Stormancer server connections.
		/// </summary>
		std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(10000);

#if !defined(_WIN32)
		/// <summary>
		/// Certificate chain to enable HTTPS connection to the stormancer server.
		/// </summary>
		/// <remarks>
		/// Set this if you use certificates for your Stormancer server HTTPS setup
		/// that are not trusted by default on your platform.
		/// This feature is not supported on Windows and Xbox.
		/// </remarks>
		std::vector<std::string> endpointRootCertificates;
#endif

		/// <summary>
		/// On platforms that need specific initialization for their network libraries,
		/// set whether the Stormancer client should perform this initialization.
		/// </summary>
		/// <remarks>
		/// Typically, you should set this to <c>false</c> if your application directly calls system network libraries,
		/// and initializes them before initializing the Stormancer client library.
		/// </remarks>
		bool shoudInitializeNetworkLibraries = true;

		/// <summary>
		/// Set whether the connection to the Stormancer server should be encrypted.
		/// </summary>
		bool encryptionEnabled = false;

		/// <summary>
		/// If using the P2P tunnel, set the port the tunnel will be bound to for clients.
		/// </summary>
		/// <remarks>
		/// This will have no effect on P2P hosts, as multiple clients can be connected to them.
		/// </remarks>
		uint16 tunnelPort = 0;

		/// <summary>
		/// Whether the tunnel's socket should use IPv4 or IPv6. NOTE: IPv6 support is experimental.
		/// </summary>
		/// <remarks>
		/// This should match the IP version used by the game's socket when sending data to the tunnel.
		/// Note that binding the tunnel's socket will fail if you set this to true on a platform that has no IPv6 support.
		/// </remarks>
		bool useIpv6Tunnel = false;

#pragma endregion

	private:

#pragma region private_methods

		// Constructor.
		Configuration(const std::string& endpoint, const std::string& account, const std::string& application, const char* headersVersion);

		// Copy constructor deleted.
		Configuration(const Configuration& other) = delete;

		// Assignment operator deleted.
		Configuration& operator=(const Configuration& other) = delete;

		static std::shared_ptr<Configuration> createInternal(const std::string& endpoint, const std::string& account, const std::string& application, const char* headersVersion);
#pragma endregion

#pragma region private_members
		/// Gets or Sets the dispatcher to be used by the client.
		std::function<std::shared_ptr<IPacketDispatcher>(const DependencyScope&)> _dispatcher;

		/// Gets or sets the transport to be used by the client.
		std::function<std::shared_ptr<ITransport>(const DependencyScope&)> _transportFactory;

		std::vector<IPlugin*> _plugins;

		/// A string containing the target server endpoint.
		std::vector<std::string> _serverEndpoints;

		static const std::function<std::shared_ptr<ITransport>(const DependencyScope&)> _defaultTransportFactory;

		/// The scheduler used by the client to run the transport and other repeated tasks.
		std::shared_ptr<IScheduler> _scheduler;

		/// The version string in STORM_VERSION, that is set in the header files. Used to compare the headers version to the build version,
		/// to signal a possible mismatch between the two. 
		const char* _headersVersion;

#pragma endregion
	};
}
