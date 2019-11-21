#pragma once
#include <string>
#include <array>
#include "stormancer/StormancerTypes.h"
#include "stormancer/Event.h"
#include "stormancer/Streams/ibytestream.h"
namespace Stormancer
{
	struct EntityId
	{
		/// <summary>
		/// The view the entity belongs to.
		/// </summary>
		std::string view;

		/// <summary>
		/// Id of the entity
		/// </summary>
		std::string id;
	};
	/// <summary>
	/// Represents an entity in the replication system
	/// </summary>
	class Entity
	{
	public:
		/// <summary>
		/// Id of the entity
		/// </summary>
		EntityId id;
		
		/// <summary>
		/// Data associated with the entity
		/// </summary>
		std::vector<byte> data;

	};

	/// <summary>
	/// Represents a view in the replication system
	/// </summary>
	class View
	{
	public:
		std::string id;



	};

	/// <summary>
	/// Replication API
	/// </summary>
	/// <remarks>
	/// The replication API provides events and functions to plug into Stormancer backed in replication system
	/// </remarks>
	class Replication
	{
	public:
		/// <summary>
		/// Event fired when an entity gets added to a view
		/// </summary>
		Event<Entity> entityAdded;

		/// <summary>
		/// Event fired when an entity gets removed from a view
		/// </summary>
		Event<EntityId> entityRemoved;

		/// <summary>
		/// Event fired whenever an entity is updated
		/// </summary>
		Event<Entity> entityUpdated;

		/// <summary>
		/// Gets the views associated with the current client.
		/// </summary>
		/// <returns></returns>
		std::vector<std::shared_ptr<View>> getViews();


	};
}
