#pragma once

#include "stormancer/Event.h"
#include "GameFinder/GameFinderModels.h"

namespace Stormancer
{
	/// <summary>
	/// The <c>GameFinder</c> enables parties or indivdual players to find Game Sessions according to custom server-side logic.
	/// </summary>
	class GameFinder
	{
	public:
		virtual ~GameFinder() {}

		/// <summary>
		/// Start a GameFinder query.
		/// </summary>
		/// <remarks>
		/// This method will attempt to connect to the server and the scene for the given <c>gameFinder</c> if the client is not yet connected to them.
		/// After the query has started, the server will notify you when a status update occurs.
		/// You should listen to these updates by providing callbacks to <c>subsribeGameFinderStateChanged()</c> and <c>subsribeGameFound()</c>.
		/// If you want to cancel the request, you should call <c>cancel()</c>, with the same <c>gameFinder</c> as the one passed to <c>findGame()</c>.
		/// We use this technique here instead of the more common <c>pplx::cancellation_token</c>-based one in order to support party scenarios,
		/// where a member of a party can cancel the party-wide <c>findGame</c> query even if they did not initiate it themselves.
		/// </remarks>
		/// <param name="gameFinder">Name of the server-side GameFinder to connect to.
		/// This will typically be the name of a scene, configured in the serviceLocator of the server application.</param>
		/// <param name="provider">Name of the provider to use for the given <c>gameFinder</c>.</param>
		/// <param name="json">Custom JSON data to send along the FindGame request.</param>
		/// <returns>A <c>pplx::task</c> that completes when the request is done.
		/// This task will complete when either one of the following happens:
		/// * A game is found
		/// * An error occurs on the server-side GameFinder
		/// * The request is canceled with a call to <c>cancel()</c>.</returns>
		virtual pplx::task<void> findGame(std::string gameFinder, const std::string &provider, std::string json) = 0;

		/// <summary>
		/// Cancel an ongoing <c>findGame</c> request.
		/// </summary>
		/// <remarks>
		/// You should call this method only after you have received the initial <c>GameFinderStatusChangedEvent</c> with <c>GameFinderStatus::Searching</c>,
		/// or else you might run into a race condition and the cancel request might not register.
		/// </remarks>
		/// <param name="gameFinder">Name of the GameFinder for which you want to cancel the search.</param>
		virtual void cancel(std::string gameFinder) = 0;

		/// <summary>
		/// Retrieve the current status of ongoing <c>findGame</c> requests for each GameFinder.
		/// </summary>
		/// <returns>A map with the GameFinder name as key, and the <c>findGame</c> request status as value.</returns>
		virtual std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus() = 0;

		/// <summary>
		/// Connect to the scene that contains the given GameFinder.
		/// </summary>
		/// <remarks>This will use the server application's ServiceLocator configuration to determine which scene to connect to for the given <c>gameFinderName</c>.</remarks>
		/// <param name="gameFinderName">Name of the GameFinder to connect to.</param>
		/// <returns>A <c>pplx::task</c> that completes when the connection to the scene that contains <c>gameFinderName</c> has completed.</returns>
		virtual pplx::task<void> connectToGameFinder(std::string gameFinderName) = 0;

		/// <summary>
		/// Disconnect from the scene that contains the given GameFinder.
		/// </summary>
		/// <param name="gameFinderName">Name of the GameFinder which scene you want to disconnect from.</param>
		/// <returns>A <c>pplx::task</c> that completes when the scene disconnection has completed.</returns>
		virtual pplx::task<void> disconnectFromGameFinder(std::string gameFinderName) = 0;

		/// <summary>
		/// Subscribe to <c>findGame</c> status notifications.
		/// </summary>
		/// <param name="callback">Callable object to be called when a <c>findGame</c> request status update occurs.</param>
		/// <returns>A reference-counted <c>Subscription</c> object that tracks the lifetime of the subscription.
		/// When the reference count of this object drops to zero, the subscription will be canceled.</returns>
		virtual Event<GameFinderStatusChangedEvent>::Subscription subsribeGameFinderStateChanged(std::function<void(GameFinderStatusChangedEvent)> callback) = 0;

		/// <summary>
		/// Subscribe to <c>GameFoundEvent</c> notifications.
		/// </summary>
		/// <remarks>A <c>GameFoundEvent</c> is triggered when a <c>findGame</c> request succeeds.
		/// It carries the information you need to join the game that the GameFinder has found for you.</remarks>
		/// <param name="callback">Callable object to be called when a <c>GameFoundEvent</c> occurs.</param>
		/// <returns>A reference-counted <c>Subscription</c> object that tracks the lifetime of the subscription.
		/// When the reference count of this object drops to zero, the subscription will be canceled.</returns>
		virtual Event<GameFoundEvent>::Subscription subsribeGameFound(std::function<void(GameFoundEvent)> callback) = 0;
	};
}