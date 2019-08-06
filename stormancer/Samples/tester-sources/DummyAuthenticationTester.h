#pragma once

#include "IAuthenticationTester.h"

/// <summary>
/// This file should be included in a cpp file for platforms that have no specific authentication method in place (yet),
/// for instance Linux.
/// Please do not include it in a cpp file that is inside tester-sources, as it would break platforms that already have an IAuthenticationTester implementation.
/// </summary>

class DummyAuthenticationTester : public IAuthenticationTester
{
	void runTests(const Stormancer::Tester&) override
	{

	}
};

std::unique_ptr<IAuthenticationTester> IAuthenticationTester::create()
{
	return std::unique_ptr<DummyAuthenticationTester>(new DummyAuthenticationTester);
}