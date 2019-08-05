#pragma once
#include <memory>

namespace Stormancer
{
	class Tester;
}

class IAuthenticationTester
{
public:
	static std::unique_ptr<IAuthenticationTester> create();

	virtual ~IAuthenticationTester() = default;

	virtual void runTests(const Stormancer::Tester&) = 0;
};