#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Utilities/TypeReflection.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/Exceptions.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>

/// \file
/// This is the dependency injection mechanism used by Stormancer.
/// For a comprehensive, step-by-step guide, see <a href="http://stormancer-docs.azurewebsites.net/cpp-client/dependency-injection.html">this page</a>.

namespace Stormancer
{
	class DependencyScope;
	class DependencyScopeImpl;

	enum class DependencyLifetime
	{
		InstancePerRequest,
		InstancePerScope,
		InstancePerMatchingScope,
		SingleInstance
	};

	using RegistrationId = uint64;

	struct RegistrationData
	{
		std::function<std::shared_ptr<void>(const DependencyScope&)> factory;
		RegistrationId id;
		std::vector<uint64> typesRegisteredAs;
		DependencyLifetime lifetime = DependencyLifetime::InstancePerRequest;
		std::string scopeTagToMatch;
		uint64 actualType;
	};

	/// <summary>
	/// The RegistrationHandle allows configuring a dependency added via <c>ContainerBuilder::registerDependency()</c>.
	/// </summary>
	/// <remarks>
	/// This object is not intended to be stored; instead, it should be seen as a handle to tweak a registration's settings.
	/// </remarks>
	/// <example>
	/// <code>
	/// ContainerBuilder builder;
	/// builder.registerDependency(fooFactory).as&lt;Foo&gt;().as&lt;Bar&gt;().singleInstance();
	/// </code>
	/// </example>
	template<typename T>
	class RegistrationHandle
	{
	public:
		/// <summary>
		/// Register this dependency as a given type.
		/// </summary>
		/// <remarks>
		/// You can call this method with as many types as you like, as long as the actual type of the dependency
		/// can be converted to <c>TAs*</c> (this is checked statically, and generates a compile error if it cannot).
		/// Dependencies are registered as the type given to <c>ContainerBuilder::registerDependency()</c> by default.
		/// This method allows to override this behavior, and add more types to register the dependency as.
		/// </remarks>
		/// <typeparam name="TAs">Type to register this dependency as.</typeparam>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		template<typename TAs>
		RegistrationHandle& as()
		{
			static_assert(std::is_convertible<T*, TAs*>::value, "You tried to register a dependency as a type it cannot be converted to.");

			uint64 typeHash = getTypeHash<TAs>();
			if (std::find(_data.typesRegisteredAs.begin(), _data.typesRegisteredAs.end(), typeHash) == _data.typesRegisteredAs.end())
			{
				_data.typesRegisteredAs.push_back(typeHash);
			}

			return *this;
		}

		/// <summary>
		/// Register the dependency as its own concrete type.
		/// </summary>
		/// <remarks>
		/// This is a shortcut for <c>as&lt;T&gt;()</c>.
		/// </remarks>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		RegistrationHandle& asSelf()
		{
			return as<T>();
		}

		/// <summary>
		/// Set this dependency to have transient instances.
		/// </summary>
		/// <remarks>
		/// An instance of the dependency will be created each time the dependency is resolved, and it will not be stored.
		/// </remarks>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		RegistrationHandle& instancePerRequest() { _data.lifetime = DependencyLifetime::InstancePerRequest; return *this; }

		/// <summary>
		/// Set this dependency to have an instance per dependency scope.
		/// </summary>
		/// <remarks>
		/// An instance of the dependency will be created once for each <c>DependencyScope</c> that the dependency is resolved from.
		/// The lifetime of these instances will be bound to their owning scope's.
		/// </remarks>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		RegistrationHandle& instancePerScope() { _data.lifetime = DependencyLifetime::InstancePerScope; return *this; }

		/// <summary>
		/// Set this dependency to have only one instance.
		/// </summary>
		/// <remarks>
		/// An instance of the dependency will be created when the dependency is first resolved, in the scope where it was registered.
		/// All subsequent requests for this dependency in any child dependency scope will return this same instance.
		/// The instance's lifetime is tied to the scope where the corresponding dependency was registered.
		/// </remarks>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		RegistrationHandle& singleInstance() { _data.lifetime = DependencyLifetime::SingleInstance; return *this; }

		/// <summary>
		/// Set this dependency to use an instance per matching dependency scope.
		/// </summary>
		/// <remarks>
		/// When you resolve this dependency under a given <c>DependencyScope</c>, the resolver will look for the closest dependency scope
		/// with a tag that is equal to <c>scopeTag</c>, starting from the scope it was requested from and up the parent chain.
		/// When such a scope is found, the dependency will be instantiated for this scope if it wasn't already.
		/// </remarks>
		/// <exception cref="std::invalid_argument">If <c>scopeTag</c> is empty</exception>
		/// <param name="scopeTag">The dependency scope tag that this dependency will be bound to.</param>
		/// <returns>A reference to the same handle, to enable chaining calls in a "fluent API" fashion.</returns>
		RegistrationHandle& instancePerMatchingScope(std::string scopeTag)
		{
			if (scopeTag.empty())
			{
				throw std::invalid_argument("scopeTag must not be empty");
			}
			_data.lifetime = DependencyLifetime::InstancePerMatchingScope;
			_data.scopeTagToMatch = scopeTag;
			return *this;
		}

	private:

		RegistrationHandle(RegistrationData& data)
			: _data(data)
		{}

		friend class ContainerBuilder;

		RegistrationData& _data;
	};


	/// <summary>
	/// The ContainerBuilder is the primary element of the dependency injection mechanism.
	/// Use it to set up the dependencies that you want to make available to consumer components.
	/// </summary>
	/// <remarks>
	/// Objects of this class are not thread-safe.
	/// </remarks>
	/// <example>
	/// <code>
	/// ContainerBuilder builder;
	/// builder.registerDependency&lt;Foo&gt;(fooFactory);
	/// builder.registerDependency&lt;Bar&gt;(barFactory);
	/// DependencyScope rootScope = builder.build();
	/// </code>
	/// </example>
	class ContainerBuilder
	{
	public:

		ContainerBuilder() : ContainerBuilder(0) {}

		/// <summary>
		/// Register a dependency into the container.
		/// </summary>
		/// <typeparam name="T">
		/// Type of the dependency to be registered.
		/// This is the concrete, actual type of the dependency.
		/// If you want this dependency to be registered as one or more types different from <c>T</c>, use <c>RegistrationHandle::as()</c>.
		/// </typeparam>
		/// <param name="factory">
		/// Factory function for the dependency.
		/// This function will be called when an instance of the dependency needs to be created. It may be called from mutliple threads at the same time.
		/// Dependencies are always instantiated lazily; that is, only when code that needs them directly or indirectly 
		/// through a <c>DependencyScope::resolve()</c> or <c>DependencyScope::resolveAll()</c> call.
		/// For details about the lifetime of dependency instances, see <c>RegistrationHandle</c>.
		/// </param>
		/// <returns>
		/// A <c>RegistrationHandle</c> that can be used to configure the dependency. See the API documentation for <c>RegistrationHandle</c> for details.
		/// </returns>
		template<typename T>
		RegistrationHandle<T> registerDependency(std::function<std::shared_ptr<T>(const DependencyScope&)> factory)
		{
			_registrations.emplace_back();
			RegistrationData& data = _registrations.back();
			data.factory = factory;
			data.id = _registrationCounter;
			data.actualType = getTypeHash<T>();
			++_registrationCounter;
			return RegistrationHandle<T>(data);
		}

		/// <summary>
		/// Register an existing instance of type <c>T</c> as a dependency.
		/// </summary>
		/// <remarks>
		/// This object will be registered with the "single instance" lifetime.
		/// Choosing a different lifetime will have no observable effect.
		/// </remarks>
		/// <typeparam name="T">Type of the object pointed to by <c>instance</c>.</typeparam>
		/// <param name="instance">Instance of type <c>T</c> to register.</param>
		/// <returns>
		/// A <c>RegistrationHandle</c> that can be used to configure the dependency. See the API documentation for <c>RegistrationHandle</c> for details.
		/// </returns>
		template<typename T>
		RegistrationHandle<T> registerDependency(std::shared_ptr<T> instance)
		{
			return registerDependency<T>([instance](const DependencyScope&) { return instance; }).singleInstance();
		}

		/// <summary>
		/// Register a dependency of type <c>T</c> with an automatically generated factory.
		/// </summary>
		/// <remarks>
		/// <c>T</c> must have a public constructor that takes arguments of types <c>TCtorArgs</c> as <c>std::shared_ptr</c>s,
		/// in the same order as they are supplied to this method.
		/// When <c>T</c> is resolved in a given scope, dependencies of types <c>TCtorArgs</c> will also be resolved from this scope
		/// and passed to <c>T</c>'s constructor.
		/// If <c>T</c>'s constructor takes a vector of dependencies of a certain type <c>U</c> (<c>std::vector&lt;std::shared_ptr&lt;U&gt;&gt;</c>),
		/// you should pass a <c>ContainerBuilder::All&lt;U&gt;</c> type argument.
		/// </remarks>
		/// <typeparam name="T">Concrete type of the dependency to be registered.</typeparam>
		/// <typeparam name="TCtorArgs">Types of the arguments for <c>T</c>'s constructor.</typeparam>
		/// <returns>
		/// A <c>RegistrationHandle</c> that can be used to configure the dependency. See the API documentation for <c>RegistrationHandle</c> for details.
		/// </returns>
		template<typename T, typename... TCtorArgs>
		RegistrationHandle<T> registerDependency();

		/// <summary>
		/// A "type tag" struct to be used as a type argument to <c>registerDependency()</c> when denoting a dependency on multiple instances of <c>T</c>.
		/// </summary>
		/// <seealso cref="registerDependency()"/>
		template<typename T> struct All {};

		/// <summary>
		/// Build a <c>DependencyScope</c> from this container.
		/// </summary>
		/// <remarks>
		/// Call this method when you are done registering and configuring dependencies.
		/// </remarks>
		/// <returns>
		/// A <c>DependencyScope</c> containing the registrations that were added to this ContainerBuilder.
		/// </returns>
		DependencyScope build();

	private:

		friend class DependencyScopeImpl;

		ContainerBuilder(RegistrationId baseId) : _registrationCounter(baseId) {}

		template<typename T>
		struct CtorResolver
		{
			static std::shared_ptr<T> resolve(const DependencyScope& scope)
			{
				return scope.resolve<T>();
			}
		};

		template<typename T>
		struct CtorResolver<All<T>>
		{
			static std::vector<std::shared_ptr<T>> resolve(const DependencyScope& scope)
			{
				return scope.resolveAll<T>();
			}
		};

		RegistrationId _registrationCounter;
		std::vector<RegistrationData> _registrations;
	};

	/// <summary>
	/// An object from which dependencies can be retrieved.
	/// </summary>
	/// <remarks>
	/// The DependencyScope controls the lifetime of the dependencies that are accessible through it.
	/// For more details on dependency lifetime, see <c>RegistrationHandle</c>.
	/// Objects of this class are thread-safe.
	/// They are not copyable so as to avoid the emergence of circualr dependencies between a DependencyScope and the dependencies which instances live in its scope.
	/// If you need to store a copy of a DependencyScope inside an object, what you most likely want is to store a child scope of this scope instead.
	/// </remarks>
	/// <example>
	/// <code>
	/// ContainerBuilder buider;
	/// builder.registerDependency&lt;Foo&gt;();
	/// DependencyScope scope = builder.build();
	/// std::shared_ptr&lt;Foo&gt; foo = scope.resolve&lt;Foo&gt;();
	/// </code>
	/// </example>
	class DependencyScope
	{
	public:

		/// <summary>
		/// Create an empty DependencyScope.
		/// </summary>
		/// <remarks>
		/// This scope will be an empty placeholder; a call to <c>isValid()</c> will return <c>false</c>.
		/// </remarks>
		DependencyScope() = default;

		// Deleted to prevent the possibility of creating circular dependencies.
		// If you want to copy the scope, what you actually want is most likely to create a child scope instead.
		DependencyScope(const DependencyScope&) = delete;
		DependencyScope& operator=(const DependencyScope&) = delete;

		DependencyScope(DependencyScope&&);
		DependencyScope& operator=(DependencyScope&&);

		/// <summary>
		/// Retrieve the dependency that was registered for the type <c>T</c>.
		/// </summary>
		/// <remarks>
		/// If needed, the dependency will be instantiated according to its lifetime options.
		/// See <c>RegistrationHandle</c> for more details on those.
		/// </remarks>
		/// <exception cref="Stormancer::DependencyResolutionException">
		/// If no dependency registration could be found for type <c>T</c>, or if the registration could not be instantiated.
		/// The exception's message will contain details about the error.
		/// </exception>
		/// <returns>A <c>shared_ptr</c> containing the instance of the dependency that was resolved.</returns>
		template<typename T>
		std::shared_ptr<T> resolve() const
		{
			uint64 typeHash = getTypeHash<T>();
			auto instance = resolveInternal(typeHash);
			return std::static_pointer_cast<T>(instance);
		}

		/// <summary>
		/// Retrieve all the dependencies that were registered for the type <c>T</c>.
		/// </summary>
		/// <remarks>
		/// If needed, the dependencies will be instantiated according to their lifetime options.
		/// See <c>RegistrationHandle</c> for more details on those.
		/// </remarks>
		/// <returns>
		/// A <c>vector</c> containing the instances of the dependencies that were resolved.
		/// The vector may be empty if no suitable dependency registration could be found, or if their lifetime requirements could not be satisfied.
		/// </returns>
		template<typename T>
		std::vector<std::shared_ptr<T>> resolveAll() const
		{
			uint64 typeHash = getTypeHash<T>();
			auto instances = resolveAllInternal(typeHash);
			
			std::vector<std::shared_ptr<T>> results;
			results.reserve(instances.size());
			std::transform(instances.begin(), instances.end(), std::back_inserter(results), [](const std::shared_ptr<void>& ptr) { return std::static_pointer_cast<T>(ptr); });
			return results;
		}

		/// <summary>
		/// Create a child dependency scope.
		/// </summary>
		/// <returns>A new DependencyScope that is a child of <c>this</c>.</returns>
		DependencyScope beginLifetimeScope() const
		{
			return beginLifetimeScope("", std::function<void(ContainerBuilder&)>{});
		}

		/// <summary>
		/// Create a child dependency scope.
		/// </summary>
		/// <param name="builder">Function in which you can register dependencies for the child scope using the supplied <c>ContainerBuilder</c> argument.</param>
		/// <returns>A new DependencyScope that is a child of <c>this</c>.</returns>
		DependencyScope beginLifetimeScope(std::function<void(ContainerBuilder&)> builder) const
		{
			return beginLifetimeScope("", builder);
		}

		/// <summary>
		/// Create a child dependency scope.
		/// </summary>
		/// <param name="tag">Tag of the child scope. This is useful if you use the <c>RegistrationHandle::instancePerMatchingScope()</c> feature.</param>
		/// <returns>A new DependencyScope that is a child of <c>this</c>.</returns>
		DependencyScope beginLifetimeScope(std::string tag) const
		{
			return beginLifetimeScope(tag, std::function<void(ContainerBuilder&)>{});
		}

		/// <summary>
		/// Create a child dependency scope.
		/// </summary>
		/// <param name="tag">Tag of the child scope. This is useful if you use the <c>RegistrationHandle::instancePerMatchingScope()</c> feature.</param>
		/// <param name="builder">Function in which you can register dependencies for the child scope using the supplied <c>ContainerBuilder</c> argument.</param>
		/// <returns>A new DependencyScope that is a child of <c>this</c>.</returns>
		DependencyScope beginLifetimeScope(std::string tag, std::function<void(ContainerBuilder&)> builder) const;

		/// <summary>
		/// Check if this scope has been fully constructed.
		/// </summary>
		/// <returns>
		/// <c>true</c> if this scope was constructed from a call to <c>ContainerBuilder::build()</c> or <c>beginLifetimeScope()</c>;
		/// <c>false</c> if it was default-constructed.
		/// </returns>
		bool isValid() const;

	private:

		friend class DependencyScopeImpl;
		friend class ContainerBuilder;

		DependencyScope(const ContainerBuilder& builder, std::string tag, std::shared_ptr<DependencyScopeImpl> parent);

		void throwIfNotValid() const;

		std::shared_ptr<void> resolveInternal(uint64 typeHash) const;
		std::vector<std::shared_ptr<void>> resolveAllInternal(uint64 typeHash) const;

		std::shared_ptr<DependencyScopeImpl> _impl;
	};

	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	// Definition is out-of-class to avoid incomplete type issue with DependencyScope
	template<typename T, typename... TCtorArgs>
	inline RegistrationHandle<T> ContainerBuilder::registerDependency()
	{
		return registerDependency<T>([](const DependencyScope& scope)
		{
			(void)scope; // Suppress unused parameter warning when TCtorArgs is empty
			return std::make_shared<T>(CtorResolver<TCtorArgs>::resolve(scope)...);
		});
	}
}
