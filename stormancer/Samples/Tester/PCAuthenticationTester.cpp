#include "IAuthenticationTester.h"

class PCAuthenticationTester : public IAuthenticationTester
{
	void runTests(const Stormancer::Tester&) override
	{

	}
};

std::unique_ptr<IAuthenticationTester> IAuthenticationTester::create()
{
	return std::unique_ptr<PCAuthenticationTester>(new PCAuthenticationTester);
}