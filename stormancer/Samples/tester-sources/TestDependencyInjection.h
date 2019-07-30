#pragma once

#include "stormancer/DependencyInjection.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


class A
{
public:
	virtual int id() const { return 0; }
	virtual ~A() = default;
};

class B : public A
{
public:
	int id() const override { return 1; }
};

class C : public B
{
public:
	int id() const override { return 2; }
};

class D
{
public:
	D(std::shared_ptr<A> a, std::shared_ptr<B> b) {}
};

class E
{
public:
	E(std::shared_ptr<D>, std::vector<std::shared_ptr<A>> allAs) : allAs(allAs) {}

	std::vector<std::shared_ptr<A>> allAs;
};

class TestDependencyInjection
{
public:
	static void runTests()
	{
		testSingleInstance();
		testSingleInstanceMultipleRegs();
		testInstancePerScope();
		testDefaultInstancePerRequest();
		testInstancePerMatchingScope();
		testResolveAll();
		testSameTypeInChild();
		testMultipleRegsForSameConcreteType();
		testRegisterCtor();
	}

private:
	static void assertex(bool condition, std::string message)
	{
		if (!condition)
		{
			throw std::runtime_error(message.c_str());
		}
	}

	static void testSingleInstance()
	{
		using namespace Stormancer;

		auto a = std::make_shared<A>();

		int numFactoryCalls = 0;

		ContainerBuilder builder;
		builder.registerDependency(a);
		builder.registerDependency<B>([&numFactoryCalls](const DependencyScope&) { ++numFactoryCalls; return std::make_shared<B>(); }).singleInstance();

		auto scope = builder.build();
		auto a_resolved = scope.resolve<A>();
		assertex(a_resolved == a, "Instance registered directly should be the same when resolved");
		auto b_resolved = scope.resolve<B>();
		auto b_resolved_next = scope.resolve<B>();
		assertex(b_resolved == b_resolved_next, "Dependency registered as SingleInstance should be the same instance when resolved multiple times");
		assertex(b_resolved != a_resolved, "a and b were registered as different types, resolving A and B should return different objects");

		auto child = scope.beginLifetimeScope();
		auto b_resolved_child = child.resolve<B>();
		assertex(b_resolved_child == b_resolved, "Single insntace dependency registered in parent scope should be the same instance in child scope");
		auto all_b = child.resolveAll<B>();
		assertex(all_b.size() == 1, "More than one instance for singleInstance registration");
		assertex(all_b[0] == b_resolved, "Instance returned by resolveAll is different from instance returned by resolve");

		assertex(numFactoryCalls == 1, "Single Instance factory should have been called exactly once, but was called "+std::to_string(numFactoryCalls)+" times");
	}

	static void testSingleInstanceMultipleRegs()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		int numFactoryCalls = 0;

		builder.registerDependency<B>([&numFactoryCalls](const DependencyScope&) { ++numFactoryCalls; return std::make_shared<B>(); }).as<B>().as<A>().singleInstance();

		auto scope = builder.build();
		auto a = scope.resolve<A>();
		auto b = scope.resolve<B>();
		assertex(a == b, "Dependency registered as A and B was not the same instance for A and B when resolved");

		auto child = scope.beginLifetimeScope();
		auto a_child = child.resolve<A>();
		auto b_child = child.resolve<B>();
		assertex(a_child == b_child, "Dependency registered as A and B was not the same instance for A and B when resolved");
		assertex(a == a_child, "No the same instance between parent and child for singleInstance dep");

		assertex(numFactoryCalls == 1, "Single Instance factory should have been called exactly once, but was called " + std::to_string(numFactoryCalls) + " times");
	}

	static void testInstancePerScope()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		builder.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); }).instancePerScope();

		auto scope = builder.build();
		auto a = scope.resolve<A>();
		assertex(a == scope.resolve<A>(), "There should be only one instance per scope");
		
		auto child = scope.beginLifetimeScope();
		assertex(child.resolve<A>() != a, "Instance for child scope should be different from the parent's");
		assertex(scope.resolve<A>() == a, "Instance for parent has changed after child instance was resolved");
	}

	static void testDefaultInstancePerRequest()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		builder.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); });
		builder.registerDependency<B>([](const DependencyScope&) { return std::make_shared<B>(); }).instancePerRequest();

		auto scope = builder.build();
		assertex(scope.resolve<B>() != scope.resolve<B>(), "Dependency registered as instancePerRequest should be a different instance for every resolve()");
		assertex(scope.resolve<A>() != scope.resolve<A>(), "Default behavior for registerDependency should be instancePerRequest");
	}

	static void testInstancePerMatchingScope()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		builder.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); }).instancePerMatchingScope("toto");

		auto scope = builder.build();
		try
		{
			scope.resolve<A>();
			throw std::runtime_error("Dependency A should not be available for this scope");
		}
		catch (DependencyResolutionException&) {} // Expected
		
		auto child_tagged = scope.beginLifetimeScope("toto");
		assertex(child_tagged.resolve<A>() == child_tagged.resolve<A>(), "There should be only one instance for the tagged scope");
		assertex(scope.resolveAll<A>().size() == 0, "resolve() on parent should still fail");

		auto child2 = child_tagged.beginLifetimeScope();
		auto child3 = child2.beginLifetimeScope("toto");
		assertex(child3.resolve<A>() != child_tagged.resolve<A>(), "Child scope with same tag should have its own instance, not the parent's");
		assertex(child2.resolve<A>() == child_tagged.resolve<A>(), "Instance for untagged child scope should be the one from tagged parent scope");
	}

	static void testResolveAll()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		builder.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); }).singleInstance();
		builder.registerDependency<B>([](const DependencyScope&) { return std::make_shared<B>(); }).instancePerRequest().as<A>();

		auto scope = builder.build();
		auto deps = scope.resolveAll<A>();
		assertex(deps.size() == 2, "There should be 2 instances");
		assertex(
			(deps[0]->id() == 0 && deps[1]->id() == 1) ||
			(deps[1]->id() == 0 && deps[0]->id() == 1),
			"One of the instances has the wrong type"
		);

		auto child = scope.beginLifetimeScope([](ContainerBuilder& b)
		{
			b.registerDependency<C>([](const DependencyScope&) { return std::make_shared<C>(); }).instancePerRequest().as<A>();
		});
		assertex(child.resolveAll<A>().size() == 3, "There should be 3 instances, the child dependency was not added");
	}

	static void testSameTypeInChild()
	{
		using namespace Stormancer;

		ContainerBuilder builder;
		builder.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); }).instancePerScope();
		auto parent = builder.build();
		auto child = parent.beginLifetimeScope([](ContainerBuilder& b)
		{
			b.registerDependency<A>([](const DependencyScope&) { return std::make_shared<A>(); }).instancePerScope();
		});

		assertex(child.resolveAll<A>().size() == 2, "Child should have access to both its own registration and its parent's");
		assertex(parent.resolveAll<A>().size() == 1, "Child's instance should be in the child scope, not the parent's");
	}

	static void testMultipleRegsForSameConcreteType()
	{
		using namespace Stormancer;

		auto a1 = std::make_shared<A>();
		auto a2 = std::make_shared<A>();
		ContainerBuilder builder;
		builder.registerDependency(a1);
		builder.registerDependency(a2);

		auto scope = builder.build();
		assertex(scope.resolve<A>() == a2, "When resolving a single dep, the returned dep should be the one that was last registered");
		assertex(scope.resolveAll<A>().size() == 2, "The second registration shouldn't overwrite the first one");
	}

	static void testRegisterCtor()
	{
		using namespace Stormancer;
		ContainerBuilder builder;
		builder.registerDependency<A>();
		builder.registerDependency<B>();
		builder.registerDependency<D, A, B>();

		auto scope = builder.build();
		scope.resolve<D>();

		auto child = scope.beginLifetimeScope([](ContainerBuilder& builder)
		{
			builder.registerDependency<B>().as<A>();
			builder.registerDependency<E, D, ContainerBuilder::All<A>>();
		});

		auto e = child.resolve<E>();
		assertex(e->allAs.size() == 2, "The E object should have been constructed with two instances of type A, but contains only " + std::to_string(e->allAs.size()));
	}
};
